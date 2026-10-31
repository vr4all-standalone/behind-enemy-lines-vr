plugins { id("com.android.application") }
val softwareNotices = file("D:/VR4ALL-BEL-Build/quest/android/app/generated/software-notices")
val prepareSoftwareNotices = tasks.register<Copy>("prepareSoftwareNotices") {
    into(softwareNotices.resolve("notices"))
    from(rootProject.file("../LICENSE"), rootProject.file("../CREDITS.md"),
         rootProject.file("../engine-lock.json"), rootProject.file("../game-set.json"),
         rootProject.file("../docs/platform-notes.md"))
    from(rootProject.file("../licenses")) { into("licenses") }
}
android {
    namespace = "com.vrlightgunhub.bel"
    ndkVersion = "29.0.14206865"
    compileSdk { version = release(36) { minorApiLevel = 1 } }
    defaultConfig {
        applicationId = "com.vrlightgunhub.bel"
        minSdk = 29
        targetSdk = 36
        versionCode = 2
        versionName = "0.1.1-beta"
        ndk { abiFilters += "arm64-v8a" }
    }
    signingConfigs { create("local") {
        storeFile = file("D:/VR4ALL-BEL-Build/private/bel.keystore")
        storePassword = "android"
        keyAlias = "bel"
        keyPassword = "android"
    } }
    buildTypes { getByName("release") { signingConfig = signingConfigs.getByName("local"); isMinifyEnabled = false } }
    sourceSets.getByName("main").assets.srcDirs("../../assets", softwareNotices)
    sourceSets.getByName("main").jniLibs.srcDirs(providers.gradleProperty("belNativeDir").orNull ?: file("D:/VR4ALL-BEL-Build/quest/jniLibs"))
    packaging { jniLibs { useLegacyPackaging = true } }
}

layout.buildDirectory.set(file("D:/VR4ALL-BEL-Build/quest/android/app"))
tasks.named("preBuild").configure { dependsOn(prepareSoftwareNotices) }
