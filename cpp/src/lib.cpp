#include <iostream>

#include <arrow/api.h>
#include <arrow/dataset/api.h>
#include <arrow/filesystem/api.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>

#include "lib.h"

std::shared_ptr<arrow::dataset::TaggedRecordBatchIterator> kBatchIt;
std::shared_ptr<arrow::ipc::RecordBatchWriter> kWriter;
std::shared_ptr<arrow::io::FixedSizeBufferWriter> kOutStream;
std::shared_ptr<arrow::MutableBuffer> kBuffer;

arrow::Status DoStartReading(char *path, void *buffer, int64_t buffer_size) {

  kBuffer = std::make_shared<arrow::MutableBuffer>(
      static_cast<uint8_t *>(buffer), buffer_size);

  std::shared_ptr<arrow::fs::FileSystem> fs =
      std::make_shared<arrow::fs::LocalFileSystem>();
  auto selector = arrow::fs::FileSelector();
  selector.base_dir = path;
  selector.recursive = true;
  std::shared_ptr<arrow::dataset::FileFormat> format =
      std::make_shared<arrow::dataset::CsvFileFormat>();

  auto fs_dataset_options = arrow::dataset::FileSystemFactoryOptions();
  fs_dataset_options.partitioning =
      arrow::dataset::HivePartitioning::MakeFactory();
  ARROW_ASSIGN_OR_RAISE(auto dataset_factory,
                        arrow::dataset::FileSystemDatasetFactory::Make(
                            fs, selector, format, fs_dataset_options));

  auto finish_options = arrow::dataset::FinishOptions();
  finish_options.validate_fragments = false;

  auto inspect_options = arrow::dataset::InspectOptions();
  inspect_options.fragments = 0;
  finish_options.inspect_options = inspect_options;
  ARROW_ASSIGN_OR_RAISE(auto dataset, dataset_factory->Finish(finish_options));

  auto schema = dataset->schema();

  ARROW_ASSIGN_OR_RAISE(auto scanner_builder, dataset->NewScan());
  scanner_builder->UseAsync(true);
  scanner_builder->UseThreads(true);
  ARROW_ASSIGN_OR_RAISE(auto scanner, scanner_builder->Finish());
  ARROW_ASSIGN_OR_RAISE(auto it, scanner->ScanBatches());
  kBatchIt = std::make_shared<arrow::dataset::TaggedRecordBatchIterator>(
      std::move(it));

  kOutStream = std::make_shared<arrow::io::FixedSizeBufferWriter>(kBuffer);
  ARROW_ASSIGN_OR_RAISE(
      kWriter, arrow::ipc::MakeStreamWriter(kOutStream, dataset->schema()));

  return arrow::Status::OK();
}

void StartReading(char *path, void *buffer, int64_t buffer_size) {
  auto status = DoStartReading(path, buffer, buffer_size);
  if (!status.ok()) {
    std::cerr << status << std::endl;
  }
}

arrow::Result<bool> DoReadNextBatch() {
  if (!kBatchIt) {
    return arrow::Status::Invalid(
        "Must successfully call DoStartReading first");
  }

  ARROW_ASSIGN_OR_RAISE(auto next_batch, kBatchIt->Next());

  kOutStream->Seek(0);
  if (next_batch.record_batch) {
    kWriter->WriteRecordBatch(*next_batch.record_batch);
    return true;
  } else {
    kWriter->Close();
    kBuffer.reset();
    kWriter.reset();
    kBatchIt.reset();
    return false;
  }
}

bool ReadNextBatch() {
  auto res = DoReadNextBatch();
  if (!res.ok()) {
    std::cerr << res.status() << std::endl;
    return false;
  }
  return *res;
}
