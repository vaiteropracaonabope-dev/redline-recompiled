#include <jni.h>
#include <string>

extern "C" {
#include "host_fs.h"
#include "host_sync.h"
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_redline_android_MainActivity_nativeRuntimeStatus(JNIEnv *env, jobject) {
#if defined(__aarch64__)
    const char *arch = "ARM64";
#elif defined(__arm__)
    const char *arch = "ARM32";
#elif defined(__x86_64__)
    const char *arch = "x86_64";
#else
    const char *arch = "desconhecida";
#endif
    const uint64_t sync_clock = redline_sync_monotonic_ns();
    const uint64_t fs_clock = redline_fs_monotonic_ns();
    const bool host_services_ready = sync_clock != 0 && fs_clock != 0;
    std::string status = std::string(host_services_ready ? "SERVIÇOS NATIVOS ATIVOS • " : "SERVIÇOS NATIVOS COM FALHA • ") +
        arch + "\nNDK/JNI + filesystem POSIX + sincronização + backend de threads incluídos.\n" +
        "O executor Xbox e o renderizador Android ainda não estão integrados.";
    return env->NewStringUTF(status.c_str());
}
