require chromium.inc
require chromium-unbundle.inc
require gn-utils.inc

GTKIC_VERSION = "${@bb.utils.contains('PACKAGECONFIG', 'gtk4', '4', '3',d)}"

inherit features_check gtk-icon-cache qemu rust-common

# The actual directory name in out/ is irrelevant for GN.
OUTPUT_DIR = "out/Release"
B = "${S}/${OUTPUT_DIR}"

# Backported patches.
# None currently \o/

# Non-specific patches.
SRC_URI += "\
    file://0001-Drop-GN-compiler-settings-conflicting-with-OE.patch \
    file://0002-v8-qemu-wrapper.patch \
    file://0003-wrapper-extra-flags.patch \
    file://0004-Delete-compiler-options-not-available-in-release-ver.patch \
    file://0005-avoid-link-latomic-failure-on-CentOS-8-host.patch \
    file://0006-Don-t-pass-unknown-LLVM-options.patch \
    file://0007-Fix-constexpr-variable-must-be-initialized-by-a-cons.patch \
    file://0008-Use-the-correct-path-to-libclang_rt.builtins.a.patch \
    file://0009-Adjust-the-Rust-build-to-our-needs.patch \
    file://0010-Don-t-require-profiler_builtins.rlib.patch \
    file://0011-Revert-Allow-and-use-std-hardware_destructive_interf.patch \
    file://0012-Revert-Set-Rust-symbol-visibility-to-hidden-when-C-s.patch \
    file://0013-pdfium-Fix-missing-PDFiumAPIStringBufferAdapter-temp.patch \
    file://0014-Revert-Remove-libavif-based-AVIF-decoder.patch \
    file://0015-Revert-Remove-third_party-libavif.patch \
    file://0016-Disable-crabbyavif-to-fix-build-errors.patch \
    file://0017-rust-Use-adler-instead-of-adler2.patch \
    file://0018-third_party-node-update_node_binaries-Update-nodejs-.patch \
"

# Missing third_party sources.
SRC_URI += "\
    git://chromium.googlesource.com/external/github.com/AOMediaCodec/libavif;protocol=https;branch=main;name=libavif;destsuffix=third_party/libavif/src \
"

SRCREV_FORMAT .= "_libavif"
SRCREV_libavif = "e7b34a1f5e9f7024d08311c7bae156061b889882"

# ARM/AArch64-specific patches.
SRC_URI:append:aarch64 = "${@bb.utils.contains('TUNE_FEATURES', 'crypto', '', ' file://arm/0001-Fix-AES-crypto-SIGILL-on-rpi4-64.patch', d)}"

# musl-specific patches.
SRC_URI:append:libc-musl = "\
    file://musl/0001-mallinfo-implementation-is-glibc-specific.patch \
    file://musl/0002-Define-TEMP_FAILURE_RETRY-and-__si_fields.patch \
    file://musl/0003-breakpad-Fix-build-with-musl.patch \
    file://musl/0004-fontconfig-Musl-does-not-have-rand_r-API.patch \
    file://musl/0005-__libc_malloc-is-internal-to-glibc.patch \
    file://musl/0006-debug-Fix-build-with-musl.patch \
    file://musl/0007-mallopt-is-glibc-specific-API.patch \
    file://musl/0008-Use-monotonic-clock-for-pthread_cond_timedwait-with-.patch \
    file://musl/0009-Fix-tab-crashes-on-musl.patch \
    file://musl/0010-pthread_getname_np.patch \
    file://musl/0011-sys-stat.patch \
    file://musl/0012-credentials-header.patch \
    file://musl/0013-fstatat64.patch \
    file://musl/0014-no-res-ninit-nclose.patch \
    file://musl/0015-fix-libc-version-include.patch \
"

ANY_OF_DISTRO_FEATURES = "opengl vulkan"

# Append instead of assigning; the gtk-icon-cache class inherited above also
# adds packages to DEPENDS.
DEPENDS += " \
    ${@bb.utils.contains('DISTRO_FEATURES', 'alsa', 'alsa-lib', '', d)} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'x11', 'at-spi2-core', '', d)} \
    atk \
    bison-native \
    cairo \
    curl \
    expat \
    flac \
    glib-2.0 \
    gn-native \
    gperf-native \
    gtk+3 \
    jpeg \
    libdrm \
    libffi \
    libstd-rs \
    libwebp \
    libxkbcommon \
    libxslt \
    ninja-native \
    nodejs-native \
    nspr \
    nspr-native \
    nss \
    nss-native \
    pango \
    pciutils \
    pkgconfig-native \
    ${@bb.utils.contains('DISTRO_FEATURES', 'pulseaudio', 'pulseaudio', '', d)} \
    qemu-native \
    rust-native \
    virtual/libgl \
"
DEPENDS:append:runtime-llvm = " compiler-rt compiler-rt-native libcxx-native"
DEPENDS:append:libc-musl = " libexecinfo"

LDFLAGS:append:libc-musl = " -lexecinfo"

# The wrapper script we use from upstream requires bash.
RDEPENDS:${PN} = "bash"

COMPATIBLE_MACHINE = "(-)"
COMPATIBLE_MACHINE:aarch64 = "(.*)"
COMPATIBLE_MACHINE:armv6 = "(.*)"
COMPATIBLE_MACHINE:armv7a = "(.*)"
COMPATIBLE_MACHINE:armv7ve = "(.*)"
COMPATIBLE_MACHINE:x86-64 = "(.*)"

# Also build the parts that are run on the host with clang.
BUILD_AR:toolchain-clang = "llvm-ar"
BUILD_CC:toolchain-clang = "clang"
BUILD_CXX:toolchain-clang = "clang++"
BUILD_LD:toolchain-clang = "clang"

PACKAGECONFIG ??= "upower use-egl use-v4l2"

# this makes sure the dependencies for the EGL mode are present; otherwise, the configure scripts
# automatically and silently fall back to GLX
PACKAGECONFIG[use-egl] = ",,virtual/egl virtual/libgles2"

# Empty PACKAGECONFIG options listed here to avoid warnings.
# The .bb file should use these to conditionally add patches
# and command-line switches (extra dependencies should not
# be necessary but are OK to add).
PACKAGECONFIG[component-build] = ""

# Starting with M61, Chromium defaults to building with its own copy of libc++
# instead of the system's libstdc++. Add a knob to control this behavior
# https://groups.google.com/a/chromium.org/d/msg/chromium-packagers/8aYO3me2SCE/SZ8pJXhZAwAJ
PACKAGECONFIG[custom-libcxx] = "use_custom_libcxx=true,use_custom_libcxx=false,,"

PACKAGECONFIG[cups] = "use_cups=true,use_cups=false,cups"
PACKAGECONFIG[gtk4] = ""
PACKAGECONFIG[kiosk-mode] = ""
PACKAGECONFIG[proprietary-codecs] = ' \
        ffmpeg_branding="Chrome" proprietary_codecs=true, \
        ffmpeg_branding="Chromium" proprietary_codecs=false \
'
# Enable UPower by default.
# Chromium expects to be able to query battery status through D-Bus.
PACKAGECONFIG[upower] = ",,,upower"

# Disable VA-API by default. It is compile time enabled since M88, but it's not
# desired to make all the users of the Chromium meta-browser recipe depend on
# libva without a real need. Both X11 (non-Ozone) and Wayland (Ozone) can use it,
# but remember to also use proprietary codecs so that H.264 is supported. Also note
# that not all the hardware configs might be supported.
PACKAGECONFIG[use-vaapi] = "use_vaapi=true use_libgav1_parser=true,use_vaapi=false,libva"
# Enable stateless V4L2 M2M video decoding support.
# This requires 'proprietary-codecs' PACKAGECONFIG
# to decode h264 streams on the V4L2 M2M device.
PACKAGECONFIG[use-v4l2] = "use_v4l2_codec=true,use_v4l2_codec=false"

# Base GN arguments, mostly related to features we want to enable or disable.
GN_ARGS = " \
        ${PACKAGECONFIG_CONFARGS} \
        is_component_build=${@bb.utils.contains('PACKAGECONFIG', 'component-build', 'true', 'false', d)} \
        use_gnome_keyring=false \
        use_kerberos=false \
        use_alsa=${@bb.utils.contains('DISTRO_FEATURES', 'alsa', 'true', 'false', d)} \
        use_pulseaudio=${@bb.utils.contains('DISTRO_FEATURES', 'pulseaudio', 'true', 'false', d)} \
        use_system_libjpeg=true \
"
# Not enabled by default, because the freetype version from oe-core
# often isn't compatible, e.g. freetype-2.11 isn't compatible with
# chromium-94
GN_ARGS += "use_system_freetype=false"

# Qt support appears to be underdevelopment in Nov 2022 so disable now
# but a PACKAGECONFIG option should be added when the feature is ready.
GN_ARGS += "use_qt=false"

# Make sure pkg-config, when used with the host's toolchain to build the
# binaries we need to run on the host, uses the right pkg-config to avoid
# passing include directories belonging to the target.
GN_ARGS += 'host_pkg_config="pkg-config-native"'

# From Chromium's BUILDCONFIG.gn:
# Set to enable the official build level of optimization. This has nothing
# to do with branding, but enables an additional level of optimization above
# release (!is_debug). This might be better expressed as a tri-state
# (debug, release, official) but for historical reasons there are two
# separate flags.
# See also: https://groups.google.com/a/chromium.org/d/msg/chromium-dev/hkcb6AOX5gE/PPT1ukWoBwAJ
GN_ARGS += "is_debug=false is_official_build=true"

# Use lld linker its quicker see https://lld.llvm.org/#performance
GN_ARGS += "use_lld=true use_gold=false"

# By default, passing is_official_build=true to GN causes its symbol_level
# variable to be set to "2". This means the compiler will be passed "-g2" and
# we will end up with a very large chrome binary (around 5Gb as of M58)
# regardless of whether DEBUG_BUILD has been set or not. In addition, binutils,
# file and other utilities are unable to read a 32-bit binary this size, which
# causes it not to be stripped.
# The solution is two-fold:
# 1. Make sure -g is not passed on 32-bit architectures via DEBUG_FLAGS. -g is
#    the same as -g2. -g1 generates an 800MB binary, which is a lot more
#    manageable.
# 2. Explicitly pass symbol_level=0 to GN. This causes -g0 to be passed
#    instead, so that if DEBUG_BUILD is not set GN will not create a huge debug
#    binary anyway. Since our compiler flags are passed after GN's, -g0 does
#    not cause any issues if DEBUG_BUILD is set, as -g1 will be passed later.
DEBUG_FLAGS:remove:arm = "-g"
DEBUG_FLAGS:append:arm = "-g1"
DEBUG_FLAGS:remove:x86 = "-g"
DEBUG_FLAGS:append:x86 = "-g1"
GN_ARGS += "symbol_level=0"

# As of Chromium 62.0.3202.94 and Yocto Rocko (GCC 7, binutils 2.29), passing
# -g to the compiler results in many linker errors on aarch64, such as:
# obj/third_party/WebKit/Source/modules/payments/libpayments.a(PaymentEventDataConversion.o)(.debug_loc+0x4e25): error: relocation overflow in R_AARCH64_ABS32
DEBUG_FLAGS:remove:aarch64 = "-g"
DEBUG_FLAGS:append:aarch64 = "-g1"

# As of Chromium 60.0.3112.101 and Yocto Pyro (GCC 6, binutils 2.28), passing
# -g to the compiler results in many linker errors on x86_64, such as:
# obj/third_party/WebKit/Source/core/loader/libloader.a(ModuleTreeLinker.o)(.debug_loc+0x1e9a5): error: relocation overflow: reference to local symbol 82 in obj/third_party/WebKit/Source/core/loader/libloader.a(ModuleTreeLinker.o)
# obj/third_party/WebKit/Source/core/libcore_generated.a(ScriptModule.o)(.debug_loc+0x253c): error: relocation overflow: reference to local symbol 31 in obj/third_party/WebKit/Source/core/libcore_generated.a(ScriptModule.o)
# so we have to use the same hack described above.
DEBUG_FLAGS:remove:x86-64 = "-g"
DEBUG_FLAGS:append:x86-64 = "-g1"

# Disable Chrome Remote Desktop (aka Chromoting) support. Building host support
# (so that the machine running this recipe can be controlled remotely from
# another machine) requires additional effort to build some extra binaries,
# whereas connecting to other machines requires building and installing a
# Chrome extension (it is not clear how to do that automatically).
GN_ARGS += "enable_remoting=false"

# NaCl support depends on the NaCl toolchain that needs to be built before NaCl
# itself. The whole process is quite cumbersome so we just disable the whole
# thing for now.
GN_ARGS += "enable_nacl=false"

# We do not want to use Chromium's own Debian-based sysroots, it is easier to
# just let Chromium's build system assume we are not using a sysroot at all and
# let Yocto handle everything.
GN_ARGS += "use_sysroot=false"

# The clang version used to build this recipe may be older than upstream
# Chromium's, which can cause unexpected warnings to show up. They should not
# cause the build to fail.
GN_ARGS += "treat_warnings_as_errors=false"

# Enabling Control Flow Integrity requires more research (i.e. we need to
# figure out when it was first deemed stable by Chromium). Upstream only
# enables this for x86-64 right now.
GN_ARGS += "is_cfi=false"

# Disable activation of field trial tests that can cause problems in
# production.
# See https://groups.google.com/a/chromium.org/d/msg/chromium-packagers/ECWC57W7E0k/9Kc5UAmyDAAJ
GN_ARGS += "disable_fieldtrial_testing_config=true"

# Disable PGO optimization as generated profdata requires the same version of
# clang that upstream uses. Otherwise, clang-llvm produces the following error:
# Unsupported instrumentation profile format version.
# See https://crrev.com/c/2424669
GN_ARGS += "chrome_pgo_phase=0"

# API keys for accessing Google services. By default, we use an invalid key
# only to prevent the "you are missing an API key" infobar from being shown on
# startup.
# See https://dev.chromium.org/developers/how-tos/api-keys for more information
# on how to obtain your own keys. You can then set the variables below in
# local.conf or a .bbappend file.
GOOGLE_API_KEY ??= "invalid-api-key"
GOOGLE_DEFAULT_CLIENT_ID ??= "invalid-client-id"
GOOGLE_DEFAULT_CLIENT_SECRET ??= "invalid-client-secret"
GN_ARGS += ' \
        google_api_key="${GOOGLE_API_KEY}" \
        google_default_client_id="${GOOGLE_DEFAULT_CLIENT_ID}" \
        google_default_client_secret="${GOOGLE_DEFAULT_CLIENT_SECRET}" \
'

# Use libcxx headers for native parts
BUILD_CPPFLAGS:append:runtime-llvm = " -isysroot=${STAGING_DIR_NATIVE} -stdlib=libc++"
# Use libgcc for native parts
BUILD_LDFLAGS:append:runtime-llvm = " -rtlib=libgcc -unwindlib=libgcc -stdlib=libc++ -lc++abi -rpath ${STAGING_LIBDIR_NATIVE}"

# Toolchains we will use for the build. We need to point to the toolchain file
# we've created, set the right target architecture and make sure we are not
# using Chromium's toolchain (bundled clang, bundled binutils etc).
GN_ARGS += ' \
        custom_toolchain="//build/toolchain/yocto:yocto_target" \
        gold_path="" \
        host_toolchain="//build/toolchain/yocto:yocto_native" \
        is_clang=true \
        clang_use_chrome_plugins=false \
        target_cpu="${@gn_target_arch_name(d)}" \
        v8_snapshot_toolchain="//build/toolchain/yocto:yocto_target" \
'

# Make sure Chromium is able to find clang libraries. See
# 0008-Use-the-correct-path-to-libclang_rt.builtins.a.patch and the
# add_clang_latest and copy_clang_library tasks for more context.
GN_ARGS += ' \
        clang_base_path="${@clang_install_path(d)}" \
        clang_version="latest" \
'

# This parameter is added by limit-number-of-LTO-jobs.patch with the default value of 8,
# but we can use whatever user configured in PARALLEL_MAKE
GN_ARGS += 'max_jobs_per_link="${@oe.utils.parallel_make_argument(d, '%d')}"'

# Use our own toolchain instead of the one upstream provides. See
# //build/config/rust.gni for more details.
# Note: the value of rustc_version doesn't matter, it's only used to ensure all
# Rust code is rebuilt after updating the Rust toolchain. This is irrelevant for
# our build setup, but not setting it leads to an error.
GN_ARGS += ' \
        rust_sysroot_absolute="${STAGING_DIR_NATIVE}/usr" \
        rustc_version="custom" \
        rust_target_triple_vendor_for_target="${TARGET_VENDOR}" \
'

# This would require the Rust toolchain to use the same LLVM version as clang.
GN_ARGS += 'toolchain_supports_rust_thin_lto=false'

# ARM builds need special additional flags (see ${S}/build/config/arm.gni).
# If we do not pass |arm_arch| and friends to GN, it will deduce a value that
# will then conflict with TUNE_CCARGS and CC.
# Note that as of M61 in some corner cases parts of the build system disable
# the "compiler:arm_fpu" GN config, whereas -mfpu is always passed via ${CC}.
# We might want to rework that if there are issues in the future.
def get_compiler_flag(params, param_name, d):
    """Given a sequence of compiler arguments in |params|, returns the value of
    an option |param_name| or an empty string if the option is not present."""
    for param in params:
      if param.startswith(param_name):
        return param.split('=')[1]
    return ''

ARM_FLOAT_ABI = "${@bb.utils.contains('TUNE_FEATURES', 'callconvention-hard', 'hard', 'softfp', d)}"
ARM_FPU = "${@get_compiler_flag(d.getVar('TUNE_CCARGS').split(), '-mfpu', d)}"
ARM_TUNE = "${@get_compiler_flag(d.getVar('TUNE_CCARGS').split(), '-mcpu', d)}"
ARM_VERSION:aarch64 = "8"
ARM_VERSION:armv7a = "7"
ARM_VERSION:armv7ve = "7"
ARM_VERSION:armv6 = "6"

# GN computes and defaults to it automatically where needed
# forcing it from cmdline breaks build on places where it ends up
# overriding what GN wants
TUNE_CCARGS:remove = "-mthumb"

GN_ARGS:append:arm = ' \
        arm_float_abi="${ARM_FLOAT_ABI}" \
        arm_fpu="${ARM_FPU}" \
        arm_tune="${ARM_TUNE}" \
        arm_version=${ARM_VERSION} \
'
# tcmalloc's atomicops-internals-arm-v6plus.h uses the "dmb" instruction that
# is not available on (some?) ARMv6 models, which causes the build to fail.
GN_ARGS:append:armv6 = ' use_partition_alloc_as_malloc=false enable_backup_ref_ptr_support=false'
# The WebRTC code fails to build on ARMv6 when NEON is enabled.
# https://bugs.chromium.org/p/webrtc/issues/detail?id=6574
GN_ARGS:append:armv6 = ' arm_use_neon=false'

# Disable glibc shims on musl
# tcmalloc does not play well with musl as of M62 (and possibly earlier).
# https://github.com/gperftools/gperftools/issues/693
GN_ARGS:append:libc-musl = ' use_allocator_shim=false use_partition_alloc_as_malloc=false enable_backup_ref_ptr_support=false'

CHROMIUM_EXTRA_ARGS ?= " \
        ${@bb.utils.contains('PACKAGECONFIG', 'use-egl', '--use-angle=gles-egl', '', d)} \
        ${@bb.utils.contains('PACKAGECONFIG', 'kiosk-mode', '--kiosk --no-first-run --incognito', '', d)} \
        ${@bb.utils.contains('PACKAGECONFIG', 'gtk4', '--gtk-version=4', '', d)} \
        ${@bb.utils.contains('PACKAGECONFIG', 'use-v4l2', '--ozone-platform-hint=wayland --enable-features=AcceleratedVideoDecoder,AcceleratedVideoDecodeLinuxGL,AcceleratedVideoDecodeLinuxZeroCopyGL', '', d)} \
"

# V8's JIT infrastructure requires binaries such as mksnapshot and
# mkpeephole to be run in the host during the build. However, these
# binaries must have the same bit-width as the target (e.g. a x86_64
# host targeting ARMv6 needs to produce a 32-bit binary). Instead of
# depending on a third Yocto toolchain, we just build those binaries
# for the target and run them on the host with QEMU.
python do_create_v8_qemu_wrapper () {
    """Creates a small wrapper that invokes QEMU to run some target V8 binaries
    on the host."""
    qemu_libdirs = [d.expand('${STAGING_DIR_HOST}${libdir}'),
                    d.expand('${STAGING_DIR_HOST}${base_libdir}')]
    qemu_cmd = qemu_wrapper_cmdline(d, d.getVar('STAGING_DIR_HOST', True),
                                    qemu_libdirs)
    wrapper_path = d.expand('${B}/v8-qemu-wrapper.sh')
    with open(wrapper_path, 'w') as wrapper_file:
        wrapper_file.write("""#!/bin/sh

# This file has been generated automatically.
# It invokes QEMU to run binaries built for the target in the host during the
# build process.

%s "$@"
""" % qemu_cmd)
    os.chmod(wrapper_path, 0o755)
}
do_create_v8_qemu_wrapper[dirs] = "${B}"
addtask create_v8_qemu_wrapper after do_patch before do_configure

# Check the LLVMVERSION defined in the meta-clang layer. Given Chromium is
# developed using new C++ features, the LLVMVERSION has to be >= 14. Otherwise,
# it is not guaranteed that Chromium will compile.
python do_check_llvm_version () {
  from distutils.version import LooseVersion

  min_llvm_version = "14.0.0"
  llvm_version = d.getVar('LLVMVERSION', False)
  if not llvm_version:
    bb.warn("Unable to determine LLVM version. Chromium may not be compiled "
            "successfully if the LLVM version is below %s." % min_llvm_version)
    return

  if LooseVersion(llvm_version) < LooseVersion(min_llvm_version):
    bb.fatal("Your LLVMVERSION (%s) is lower than the minimum required "
             "LLVMVERSION (%s). If you are using dunfell, make sure you "
             "use dunfell-clang14 branch." % (llvm_version, min_llvm_version))
}
addtask check_llvm_version before do_configure

python do_write_toolchain_file () {
    """Writes a BUILD.gn file for Yocto detailing its toolchains."""
    toolchain_dir = d.expand("${S}/build/toolchain/yocto")
    bb.utils.mkdirhier(toolchain_dir)
    toolchain_file = os.path.join(toolchain_dir, "BUILD.gn")
    write_toolchain_file(d, toolchain_file)
}
addtask write_toolchain_file after do_patch before do_configure

do_add_nodejs_symlink () {
	# Adds a symlink to the node binary to the location expected by
	# Chromium's build system.
	chromium_node_dir="${S}/third_party/node/linux/node-linux-x64/bin"
	nodejs_native_path="${STAGING_BINDIR_NATIVE}/node"
	mkdir -p "${chromium_node_dir}"
	if [ ! -f "${nodejs_native_path}" ]; then
		echo "${nodejs_native_path} does not exist."
		exit 1
	fi
	ln -sf "${nodejs_native_path}" "${chromium_node_dir}/node"
}
addtask add_nodejs_symlink after do_configure before do_compile

do_add_clang_latest () {
    # Creates a `latest` directory in the native sysroot's /usr/lib/clang
    # directory that is a copy of /usr/lib/clang/$CLANG_VERSION. Chromium
    # manually links against libclang_rt.builtins.a and uses the `clang_version`
    # GN variable to find it. This allows us to set it to the same value for all
    # Yocto releases.
    cd "${STAGING_LIBDIR_NATIVE}/clang"
    rm -rf latest
    # find the directory containing the library
    for dir in *; do
        if [ -n "$(find $dir -name 'libclang_rt.builtins*')" ] ; then
            cp -r "$dir" latest
            break
        fi
    done
}
addtask add_clang_latest after do_configure before do_compile

do_copy_clang_library () {
    # Chromium needs to link against libclang_rt.builtins.a for both host and
    # target code, and expects to find both libraries in the same directory
    # (thanks to 0008-Use-the-correct-path-to-libclang_rt.builtins.a.patch).
    cd "${STAGING_DIR_HOST}${nonarch_libdir}/clang"
    # lib_file = "./$CLANG_VERSION/lib/linux/libclang_rt.builtins-$ARCH.a"
    lib_file="$(find . -name 'libclang_rt.builtins*')"
    # stripped_lib_file = "lib/linux/libclang_rt.builtins-$ARCH.a"
    stripped_lib_file="${lib_file#*/*/}"
    cp "$lib_file" "${STAGING_LIBDIR_NATIVE}/clang/latest/${stripped_lib_file}"
}
addtask copy_clang_library after do_add_clang_latest before do_compile

do_copy_target_rustlibs () {
    # Chromium needs a single Rust sysroot that contains the rustlibs for both
    # the host and target, so we copy the target rustlibs to the native sysroot.
    rustlib_src_dir="${STAGING_LIBDIR}/rustlib/${TARGET_ARCH}"*
    cp -r $rustlib_src_dir "${STAGING_LIBDIR_NATIVE}/rustlib"
}
addtask copy_target_rustlibs after do_configure before do_compile

do_copy_missing_third_party_sources () {
	rm -rf ${S}/third_party/libavif/src
	cp -r ${UNPACKDIR}/third_party/libavif/src/ ${S}/third_party/libavif/
}
addtask copy_missing_third_party_sources after do_patch before do_configure

do_configure() {
	cd ${S}
	python3 ./build/linux/unbundle/replace_gn_files.py --system-libraries ${GN_UNBUNDLE_LIBS}
	gn gen --args='${GN_ARGS}' "${OUTPUT_DIR}"
}

do_compile() {
	ninja -v ${PARALLEL_MAKE} chrome chrome_sandbox chromedriver.unstripped
}
do_compile[progress] = "outof:^\[(\d+)/(\d+)\]\s+"

do_install() {
	install -d ${D}${bindir}
	install -d ${D}${datadir}
	install -d ${D}${datadir}/applications
	install -d ${D}${datadir}/icons
	install -d ${D}${datadir}/icons/hicolor
	install -d ${D}${libdir}/chromium
	install -d ${D}${libdir}/chromium/locales

	install -m 4755 chrome_sandbox ${D}${libdir}/chromium/chrome-sandbox
	install -m 0755 chrome ${D}${libdir}/chromium/chromium-bin
	install -m 0644 *.bin ${D}${libdir}/chromium/
	install -m 0644 icudtl.dat ${D}${libdir}/chromium/icudtl.dat

	# Process and install Chromium's template .desktop file.
	sed -e "s,@@MENUNAME@@,Chromium Browser,g" \
	    -e "s,@@PACKAGE@@,chromium,g" \
	    -e "s,@@USR_BIN_SYMLINK_NAME@@,chromium,g" \
	    ${S}/chrome/installer/linux/common/desktop.template > chromium.desktop
	install -m 0644 chromium.desktop ${D}${datadir}/applications/chromium.desktop

	# Install icons.
	for size in 16 24 32 48 64 128 256; do
		install -d ${D}${datadir}/icons/hicolor/${size}x${size}
		install -d ${D}${datadir}/icons/hicolor/${size}x${size}/apps
		for dirname in "chromium" "default_100_percent/chromium"; do
			icon="${S}/chrome/app/theme/${dirname}/product_logo_${size}.png"
			if [ -f "${icon}" ]; then
				install -m 0644 "${icon}" \
					${D}${datadir}/icons/hicolor/${size}x${size}/apps/chromium.png
			fi
		done
	done

	# A wrapper for the proprietary Google Chrome version already exists.
	# We can just use that one instead of reinventing the wheel.
	WRAPPER_FILE=${S}/chrome/installer/linux/common/wrapper
	sed -e "s,@@CHANNEL@@,stable,g" \
		-e "s,@@PROGNAME@@,chromium-bin,g" \
		${WRAPPER_FILE} > chromium-wrapper
	install -m 0755 chromium-wrapper ${D}${libdir}/chromium/chromium-wrapper
	ln -s ${libdir}/chromium/chromium-wrapper ${D}${bindir}/chromium

	# Chromium *.pak files
	install -m 0644 chrome_*.pak ${D}${libdir}/chromium/
	install -m 0644 resources.pak ${D}${libdir}/chromium/resources.pak

	# Locales.
	install -m 0644 locales/*.pak ${D}${libdir}/chromium/locales/

	# Add extra command line arguments to the chromium-wrapper script by
	# modifying the dummy "CHROME_EXTRA_ARGS" line
	sed -i "s/^CHROME_EXTRA_ARGS=\"\"/CHROME_EXTRA_ARGS=\"${CHROMIUM_EXTRA_ARGS}\"/" ${D}${libdir}/chromium/chromium-wrapper

	# ANGLE.
	if [ -e libEGL.so ]; then
		install -m 0644 libEGL.so ${D}${libdir}/chromium/
		install -m 0644 libGLESv2.so ${D}${libdir}/chromium/
	fi

	# libvulkan (for ANGLE).
	if [ -e libvulkan.so.1 ]; then
		install -m 0644 libvulkan.so.1 ${D}${libdir}/chromium/
	fi

	# Swiftshader VK.
	if [ -e libvk_swiftshader.so ]; then
		install -m 0644 libvk_swiftshader.so ${D}${libdir}/chromium/
		# This is needed for ANGLE to find libvk_swiftshader.so.
		install -m 0644 vk_swiftshader_icd.json ${D}${libdir}/chromium/
	fi

	if [ -n "${@bb.utils.contains('PACKAGECONFIG', 'component-build', 'component-build', '', d)}" ]; then
		install -m 0755 *.so ${D}${libdir}/chromium/
	fi

	# When building chromium with use_system_minigbm=false,
	# libminigbm.so does not seem to get linked in statically.
	# So we simply check whether it exists in all cases and ship it.
	if [ -e libminigbm.so ]; then
		install -m 0755 libminigbm.so ${D}${libdir}/chromium/
	fi

	# ChromeDriver.
	install -m 0755 chromedriver.unstripped ${D}${bindir}/chromedriver

	# Install chrome_crashpad_handler.
	install -m 0755 chrome_crashpad_handler ${D}${libdir}/chromium/
}

PACKAGES =+ "${PN}-chromedriver"

FILES:${PN}-chromedriver = "${bindir}/chromedriver"

FILES:${PN} = " \
        ${bindir}/chromium \
        ${datadir}/applications/chromium.desktop \
        ${datadir}/icons/hicolor/*x*/apps/chromium.png \
        ${libdir}/chromium/* \
"

PACKAGE_DEBUG_SPLIT_STYLE = "debug-without-src"

# There is no need to ship empty -dev packages.
ALLOW_EMPTY:${PN}-dev = "0"

# ERROR: QA Issue: lib32-chromium-ozone-wayland: ELF binary /usr/bin/chromium has relocations in .text [textrel]
INSANE_SKIP:${PN}:append:x86 = "textrel"

