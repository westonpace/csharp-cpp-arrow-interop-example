#include <cstdint>

#define DLL_PUBLIC __attribute__((visibility("default")))

extern "C" DLL_PUBLIC void StartReading(char *path, void *buffer,
                                        int64_t buffer_size);
extern "C" DLL_PUBLIC bool ReadNextBatch();
