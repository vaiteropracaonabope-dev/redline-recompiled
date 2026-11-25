#include <jni.h>
#include <string>

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
    std::string status = std::string("PONTE NATIVA NDK ATIVA • ") + arch +
        "\nCamada JNI carregada; execução do jogo e renderizador Android ainda não integrados.";
    return env->NewStringUTF(status.c_str());
}
