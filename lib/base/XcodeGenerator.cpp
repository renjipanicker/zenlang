#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/XcodeGenerator.hpp"
#include "base/compiler.hpp"

class z::XcodeGenerator::Impl : public z::Generator::Impl {
public:
    inline Impl(const z::Ast::Project& project) : Generator::Impl(project) {}
public:
    void run();
private:
    inline void generateConfig(const z::Ast::Config& config, z::ofile& os);
    inline void generateProject();
    inline void generatePCHFile();
    inline void generatePListFile();
};

inline void z::XcodeGenerator::Impl::generateConfig(const z::Ast::Config& config, z::ofile& os) {
    z::Compiler compiler(_project, config);
    compiler.compile();
    os() << "/* Begin XCBuildConfiguration section */" << std::endl;
    os() << "        " << _project.name() << "_Debug_ProjectConfig = {" << std::endl;
    os() << "			isa = XCBuildConfiguration;" << std::endl;
    os() << "			buildSettings = {" << std::endl;
    os() << "				ALWAYS_SEARCH_USER_PATHS = NO;" << std::endl;
    os() << "               ARCHS = \"$(ARCHS_STANDARD_64_BIT)\";" << std::endl;
    os() << "               CLANG_CXX_LANGUAGE_STANDARD = \"gnu++0x\";" << std::endl;
//    os() << "               CLANG_CXX_LIBRARY = \"libc++\";" << std::endl;
    os() << "               CLANG_CXX_LIBRARY = \"libstdc++\";" << std::endl;
    os() << "				CLANG_ENABLE_OBJC_ARC = YES;" << std::endl;
    os() << "				CLANG_WARN_EMPTY_BODY = YES;" << std::endl;
    os() << "				CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;" << std::endl;
    os() << "				COPY_PHASE_STRIP = NO;" << std::endl;
    os() << "				GCC_C_LANGUAGE_STANDARD = gnu99;" << std::endl;
    os() << "				GCC_DYNAMIC_NO_PIC = NO;" << std::endl;
    os() << "				GCC_ENABLE_OBJC_EXCEPTIONS = YES;" << std::endl;
    os() << "				GCC_OPTIMIZATION_LEVEL = 0;" << std::endl;
    os() << "				GCC_PREPROCESSOR_DEFINITIONS = (" << std::endl;
    os() << "                    \"DEBUG=1\"," << std::endl;
    if(config.gui()) {
        os() << "                    \"GUI=1\"," << std::endl;
    }
    if(config.buildMode() == z::Ast::Config::BuildMode::Executable) {
        os() << "                    \"Z_EXE=1\"," << std::endl;
    }
    os() << "                    \"$(inherited)\"," << std::endl;
    os() << "				);" << std::endl;
    os() << "				GCC_SYMBOLS_PRIVATE_EXTERN = NO;" << std::endl;
    os() << "				GCC_WARN_64_TO_32_BIT_CONVERSION = YES;" << std::endl;
    os() << "				GCC_WARN_ABOUT_RETURN_TYPE = YES;" << std::endl;
    os() << "				GCC_WARN_UNINITIALIZED_AUTOS = YES;" << std::endl;
    os() << "				GCC_WARN_UNUSED_VARIABLE = YES;" << std::endl;
    os() << "				MACOSX_DEPLOYMENT_TARGET = 10.8;" << std::endl;
    os() << "				ONLY_ACTIVE_ARCH = YES;" << std::endl;
    os() << "				SDKROOT = macosx;" << std::endl;
    os() << "			};" << std::endl;
    os() << "			name = Debug;" << std::endl;
    os() << "		};" << std::endl;
    os() << "        " << _project.name() << "_Release_ProjectConfig = {" << std::endl;
    os() << "			isa = XCBuildConfiguration;" << std::endl;
    os() << "			buildSettings = {" << std::endl;
    os() << "				ALWAYS_SEARCH_USER_PATHS = NO;" << std::endl;
    os() << "               ARCHS = \"$(ARCHS_STANDARD_64_BIT)\";" << std::endl;
    os() << "               CLANG_CXX_LANGUAGE_STANDARD = \"gnu++0x\";" << std::endl;
//    os() << "               CLANG_CXX_LIBRARY = \"libc++\";" << std::endl;
    os() << "               CLANG_CXX_LIBRARY = \"libstdc++\";" << std::endl;
    os() << "				CLANG_ENABLE_OBJC_ARC = YES;" << std::endl;
    os() << "				CLANG_WARN_EMPTY_BODY = YES;" << std::endl;
    os() << "				CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;" << std::endl;
    os() << "				COPY_PHASE_STRIP = YES;" << std::endl;
    os() << "                DEBUG_INFORMATION_FORMAT = \"dwarf-with-dsym\";" << std::endl;
    os() << "				GCC_PREPROCESSOR_DEFINITIONS = (" << std::endl;
    if(config.gui()) {
        os() << "                    \"GUI=1\"," << std::endl;
    }
    if(config.buildMode() == z::Ast::Config::BuildMode::Executable) {
        os() << "                    \"Z_EXE=1\"," << std::endl;
    }
    os() << "                    \"$(inherited)\"," << std::endl;
    os() << "				);" << std::endl;
    os() << "				GCC_C_LANGUAGE_STANDARD = gnu99;" << std::endl;
    os() << "				GCC_ENABLE_OBJC_EXCEPTIONS = YES;" << std::endl;
    os() << "				GCC_WARN_64_TO_32_BIT_CONVERSION = YES;" << std::endl;
    os() << "				GCC_WARN_ABOUT_RETURN_TYPE = YES;" << std::endl;
    os() << "				GCC_WARN_UNINITIALIZED_AUTOS = YES;" << std::endl;
    os() << "				GCC_WARN_UNUSED_VARIABLE = YES;" << std::endl;
    os() << "				MACOSX_DEPLOYMENT_TARGET = 10.8;" << std::endl;
    os() << "				SDKROOT = macosx;" << std::endl;
    os() << "			};" << std::endl;
    os() << "			name = Release;" << std::endl;
    os() << "		};" << std::endl;
    os() << "        " << _project.name() << "_OSX_Debug_TargetConfig = {" << std::endl;
    os() << "			isa = XCBuildConfiguration;" << std::endl;
    os() << "			buildSettings = {" << std::endl;
    os() << "				CLANG_ENABLE_OBJC_ARC = NO;" << std::endl;
    os() << "				COMBINE_HIDPI_IMAGES = YES;" << std::endl;
    os() << "               GCC_PRECOMPILE_PREFIX_HEADER = YES;" << std::endl;
    os() << "               GCC_PREFIX_HEADER = \"" << _project.name() << "-OSX-Prefix.pch\";" << std::endl;
    os() << "               HEADER_SEARCH_PATHS = \"" << _project.zlibPath() << "\";" << std::endl;
    os() << "               INFOPLIST_FILE = \"" << _project.name() << "-OSX-Info.plist\";" << std::endl;
    os() << "               PRODUCT_NAME = \"$(TARGET_NAME)\";" << std::endl;
    os() << "				WRAPPER_EXTENSION = app;" << std::endl;
    os() << "			};" << std::endl;
    os() << "			name = Debug;" << std::endl;
    os() << "		};" << std::endl;
    os() << "        " << _project.name() << "_OSX_Release_TargetConfig = {" << std::endl;
    os() << "			isa = XCBuildConfiguration;" << std::endl;
    os() << "			buildSettings = {" << std::endl;
    os() << "				CLANG_ENABLE_OBJC_ARC = NO;" << std::endl;
    os() << "				COMBINE_HIDPI_IMAGES = YES;" << std::endl;
    os() << "				GCC_PRECOMPILE_PREFIX_HEADER = YES;" << std::endl;
    os() << "               GCC_PREFIX_HEADER = \"" << _project.name() << "-OSX-Prefix.pch\";" << std::endl;
    os() << "               HEADER_SEARCH_PATHS = \"" << _project.zlibPath() << "\";" << std::endl;
    os() << "               INFOPLIST_FILE = \"" << _project.name() << "-OSX-Info.plist\";" << std::endl;
    os() << "               PRODUCT_NAME = \"$(TARGET_NAME)\";" << std::endl;
    os() << "				WRAPPER_EXTENSION = app;" << std::endl;
    os() << "			};" << std::endl;
    os() << "			name = Release;" << std::endl;
    os() << "		};" << std::endl;
    os() << "        " << _project.name() << "_IOS_Debug_TargetConfig = {" << std::endl;
    os() << "			isa = XCBuildConfiguration;" << std::endl;
    os() << "			buildSettings = {" << std::endl;
    os() << "                \"CODE_SIGN_IDENTITY[sdk=iphoneos*]\" = \"iPhone Developer\";" << std::endl;
    os() << "				CLANG_ENABLE_OBJC_ARC = NO;" << std::endl;
    os() << "				FRAMEWORK_SEARCH_PATHS = (" << std::endl;
    os() << "                    \"$(inherited)\"," << std::endl;
    os() << "                    \"\\\"$(SYSTEM_APPS_DIR)/Xcode.app/Contents/Developer/Library/Frameworks\\\"\"," << std::endl;
    os() << "				);" << std::endl;
    os() << "				GCC_PRECOMPILE_PREFIX_HEADER = YES;" << std::endl;
    os() << "               GCC_PREFIX_HEADER = \"" << _project.name() << "-IOS-Prefix.pch\";" << std::endl;
    os() << "               HEADER_SEARCH_PATHS = \"" << _project.zlibPath() << "\";" << std::endl;
    os() << "               INFOPLIST_FILE = \"" << _project.name() << "-IOS-Info.plist\";" << std::endl;
    os() << "				IPHONEOS_DEPLOYMENT_TARGET = 6.0;" << std::endl;
    os() << "               PRODUCT_NAME = \"$(TARGET_NAME)\";" << std::endl;
    os() << "				SDKROOT = iphoneos;" << std::endl;
    os() << "               TARGETED_DEVICE_FAMILY = \"1,2\";" << std::endl;
    os() << "				WRAPPER_EXTENSION = app;" << std::endl;
    os() << "			};" << std::endl;
    os() << "			name = Debug;" << std::endl;
    os() << "		};" << std::endl;
    os() << "        " << _project.name() << "_IOS_Release_TargetConfig = {" << std::endl;
    os() << "			isa = XCBuildConfiguration;" << std::endl;
    os() << "			buildSettings = {" << std::endl;
    os() << "                \"CODE_SIGN_IDENTITY[sdk=iphoneos*]\" = \"iPhone Developer\";" << std::endl;
    os() << "				CLANG_ENABLE_OBJC_ARC = NO;" << std::endl;
    os() << "				FRAMEWORK_SEARCH_PATHS = (" << std::endl;
    os() << "                    \"$(inherited)\"," << std::endl;
    os() << "                    \"\\\"$(SYSTEM_APPS_DIR)/Xcode.app/Contents/Developer/Library/Frameworks\\\"\"," << std::endl;
    os() << "				);" << std::endl;
    os() << "				GCC_PRECOMPILE_PREFIX_HEADER = YES;" << std::endl;
    os() << "               GCC_PREFIX_HEADER = \"" << _project.name() << "-IOS-Prefix.pch\";" << std::endl;
    os() << "               HEADER_SEARCH_PATHS = \"" << _project.zlibPath() << "\";" << std::endl;
    os() << "               INFOPLIST_FILE = \"" << _project.name() << "-IOS-Info.plist\";" << std::endl;
    os() << "				IPHONEOS_DEPLOYMENT_TARGET = 6.0;" << std::endl;
    os() << "               OTHER_CFLAGS = \"-DNS_BLOCK_ASSERTIONS=1\";" << std::endl;
    os() << "               PRODUCT_NAME = \"$(TARGET_NAME)\";" << std::endl;
    os() << "				SDKROOT = iphoneos;" << std::endl;
    os() << "               TARGETED_DEVICE_FAMILY = \"1,2\";" << std::endl;
    os() << "				VALIDATE_PRODUCT = YES;" << std::endl;
    os() << "				WRAPPER_EXTENSION = app;" << std::endl;
    os() << "			};" << std::endl;
    os() << "			name = Release;" << std::endl;
    os() << "		};" << std::endl;
    os() << "/* End XCBuildConfiguration section */" << std::endl;
    os() << "" << std::endl;
    os() << "/* Begin XCConfigurationList section */" << std::endl;
    os() << "        Project_ConfigList = {" << std::endl;
    os() << "			isa = XCConfigurationList;" << std::endl;
    os() << "			buildConfigurations = (" << std::endl;
    os() << "                " << _project.name() << "_Debug_ProjectConfig," << std::endl;
    os() << "                " << _project.name() << "_Release_ProjectConfig," << std::endl;
    os() << "			);" << std::endl;
    os() << "			defaultConfigurationIsVisible = 0;" << std::endl;
    os() << "			defaultConfigurationName = Release;" << std::endl;
    os() << "        };" << std::endl;
    os() << "        OSX_Target_ConfigList = {" << std::endl;
    os() << "			isa = XCConfigurationList;" << std::endl;
    os() << "			buildConfigurations = (" << std::endl;
    os() << "                " << _project.name() << "_OSX_Debug_TargetConfig," << std::endl;
    os() << "                " << _project.name() << "_OSX_Release_TargetConfig," << std::endl;
    os() << "			);" << std::endl;
    os() << "			defaultConfigurationIsVisible = 0;" << std::endl;
    os() << "			defaultConfigurationName = Release;" << std::endl;
    os() << "        };" << std::endl;
    os() << "        IOS_Target_ConfigList = {" << std::endl;
    os() << "			isa = XCConfigurationList;" << std::endl;
    os() << "			buildConfigurations = (" << std::endl;
    os() << "                " << _project.name() << "_IOS_Debug_TargetConfig," << std::endl;
    os() << "                " << _project.name() << "_IOS_Release_TargetConfig," << std::endl;
    os() << "			);" << std::endl;
    os() << "			defaultConfigurationIsVisible = 0;" << std::endl;
    os() << "			defaultConfigurationName = Release;" << std::endl;
    os() << "        };" << std::endl;
    os() << "/* End XCConfigurationList section */" << std::endl;
}

inline void z::XcodeGenerator::Impl::generatePCHFile() {
    z::ofile os1(_gendir + "/" + _project.name() + "-OSX-Prefix.pch");
    os1() << "#ifdef __OBJC__" << std::endl;
    os1() << "#import <Cocoa/Cocoa.h>" << std::endl;
    os1() << "#endif" << std::endl;

    z::ofile os2(_gendir + "/" + _project.name() + "-IOS-Prefix.pch");
    os2() << "#ifdef __OBJC__" << std::endl;
    os2() << "#import <UIKit/UIKit.h>" << std::endl;
    os2() << "#import <Foundation/Foundation.h>" << std::endl;
    os2() << "#endif" << std::endl;
}

inline void z::XcodeGenerator::Impl::generatePListFile() {
    z::ofile os1(_gendir + "/" + _project.name() + "-OSX-Info.plist");
    os1() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    os1() << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << std::endl;
    os1() << "<plist version=\"1.0\">" << std::endl;
    os1() << "<dict>" << std::endl;
    os1() << "<key>CFBundleDevelopmentRegion</key>" << std::endl;
    os1() << "<string>en</string>" << std::endl;
    os1() << "<key>CFBundleExecutable</key>" << std::endl;
    os1() << "<string>${EXECUTABLE_NAME}</string>" << std::endl;
    os1() << "<key>CFBundleIconFile</key>" << std::endl;
    os1() << "<string></string>" << std::endl;
    os1() << "<key>CFBundleIdentifier</key>" << std::endl;
    os1() << "<string>test.${PRODUCT_NAME:rfc1034identifier}</string>" << std::endl;
    os1() << "<key>CFBundleInfoDictionaryVersion</key>" << std::endl;
    os1() << "<string>6.0</string>" << std::endl;
    os1() << "<key>CFBundleName</key>" << std::endl;
    os1() << "<string>${PRODUCT_NAME}</string>" << std::endl;
    os1() << "<key>CFBundlePackageType</key>" << std::endl;
    os1() << "<string>APPL</string>" << std::endl;
    os1() << "<key>CFBundleShortVersionString</key>" << std::endl;
    os1() << "<string>1.0</string>" << std::endl;
    os1() << "<key>CFBundleSignature</key>" << std::endl;
    os1() << "<string>\?\?\?\?</string>" << std::endl;
    os1() << "<key>CFBundleVersion</key>" << std::endl;
    os1() << "<string>1</string>" << std::endl;
    os1() << "<key>LSMinimumSystemVersion</key>" << std::endl;
    os1() << "<string>${MACOSX_DEPLOYMENT_TARGET}</string>" << std::endl;
    os1() << "<key>NSHumanReadableCopyright</key>" << std::endl;
    os1() << "<string>Copyright © 2012 Renji Panicker. All rights reserved.</string>" << std::endl;
    os1() << "<key>NSPrincipalClass</key>" << std::endl;
    os1() << "<string>NSApplication</string>" << std::endl;
    os1() << "</dict>" << std::endl;
    os1() << "</plist>" << std::endl;

    z::ofile os2(_gendir + "/" + _project.name() + "-IOS-Info.plist");

    os2() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    os2() << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << std::endl;
    os2() << "<plist version=\"1.0\">" << std::endl;
    os2() << "<dict>" << std::endl;
    os2() << "<key>CFBundleDevelopmentRegion</key>" << std::endl;
    os2() << "<string>en</string>" << std::endl;
    os2() << "<key>CFBundleDisplayName</key>" << std::endl;
    os2() << "<string>${PRODUCT_NAME}</string>" << std::endl;
    os2() << "<key>CFBundleExecutable</key>" << std::endl;
    os2() << "<string>${EXECUTABLE_NAME}</string>" << std::endl;
    os2() << "<key>CFBundleIdentifier</key>" << std::endl;
    os2() << "<string>test.${PRODUCT_NAME:rfc1034identifier}</string>" << std::endl;
    os2() << "<key>CFBundleInfoDictionaryVersion</key>" << std::endl;
    os2() << "<string>6.0</string>" << std::endl;
    os2() << "<key>CFBundleName</key>" << std::endl;
    os2() << "<string>${PRODUCT_NAME}</string>" << std::endl;
    os2() << "<key>CFBundlePackageType</key>" << std::endl;
    os2() << "<string>APPL</string>" << std::endl;
    os2() << "<key>CFBundleShortVersionString</key>" << std::endl;
    os2() << "<string>1.0</string>" << std::endl;
    os2() << "<key>CFBundleSignature</key>" << std::endl;
    os2() << "<string>\?\?\?\?</string>" << std::endl;
    os2() << "<key>CFBundleVersion</key>" << std::endl;
    os2() << "<string>1.0</string>" << std::endl;
    os2() << "<key>LSRequiresIPhoneOS</key>" << std::endl;
    os2() << "<true/>" << std::endl;
    os2() << "<key>UIRequiredDeviceCapabilities</key>" << std::endl;
    os2() << "<array>" << std::endl;
    os2() << "    <string>armv7</string>" << std::endl;
    os2() << "</array>" << std::endl;
    os2() << "<key>UISupportedInterfaceOrientations</key>" << std::endl;
    os2() << "<array>" << std::endl;
    os2() << "    <string>UIInterfaceOrientationPortrait</string>" << std::endl;
    os2() << "    <string>UIInterfaceOrientationLandscapeLeft</string>" << std::endl;
    os2() << "    <string>UIInterfaceOrientationLandscapeRight</string>" << std::endl;
    os2() << "</array>" << std::endl;
    os2() << "<key>UISupportedInterfaceOrientations~ipad</key>" << std::endl;
    os2() << "<array>" << std::endl;
    os2() << "    <string>UIInterfaceOrientationPortrait</string>" << std::endl;
    os2() << "    <string>UIInterfaceOrientationPortraitUpsideDown</string>" << std::endl;
    os2() << "    <string>UIInterfaceOrientationLandscapeLeft</string>" << std::endl;
    os2() << "    <string>UIInterfaceOrientationLandscapeRight</string>" << std::endl;
    os2() << "</array>" << std::endl;
    os2() << "</dict>" << std::endl;
    os2() << "</plist>" << std::endl;
}

inline void z::XcodeGenerator::Impl::generateProject() {
    z::dir::mkpath(_gendir + "/" + _project.name() + ".xcodeproj/");
    z::ofile os(_gendir + "/" + _project.name() + ".xcodeproj/project.pbxproj");
    os() << "// !$*UTF8*$!" << std::endl;
    os() << "{" << std::endl;
    os() << "	archiveVersion = 1;" << std::endl;
    os() << "	classes = {" << std::endl;
    os() << "	};" << std::endl;
    os() << "	objectVersion = 46;" << std::endl;
    os() << "	objects = {" << std::endl;
    os() << "/* Begin PBXBuildFile section */" << std::endl;
    os() << "        Sqlite3_OSX_Framework_BuildFile = {isa = PBXBuildFile; fileRef = Sqlite3_OSX_Framework_FileRef; };" << std::endl;
    os() << "        Sqlite3_IOS_Framework_BuildFile = {isa = PBXBuildFile; fileRef = Sqlite3_IOS_Framework_FileRef; };" << std::endl;
    os() << "        WebKit_Framework_BuildFile = {isa = PBXBuildFile; fileRef = WebKit_Framework_FileRef; };" << std::endl;
    os() << "        Cocoa_Framework_BuildFile = {isa = PBXBuildFile; fileRef = Cocoa_Framework_FileRef; };" << std::endl;
    os() << "        UIKit_Framework_BuildFile = {isa = PBXBuildFile; fileRef = UIKit_Framework_FileRef; };" << std::endl;
    os() << "        Foundation_Framework_BuildFile = {isa = PBXBuildFile; fileRef = Foundation2_Framework_FileRef; };" << std::endl;
    os() << "        CoreGraphics_Framework_BuildFile = {isa = PBXBuildFile; fileRef = CoreGraphics_Framework_FileRef ; };" << std::endl;
    os() << "        zenlang_cpp_BuildFile = {isa = PBXBuildFile; fileRef = zenlang_cpp_FileRef; settings = {COMPILER_FLAGS = \"-x objective-c++\"; }; };" << std::endl;
    for(FileList::const_iterator it = _hppFileList.begin(), ite = _hppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_hpp_BuildFile = {isa = PBXBuildFile; fileRef = " << basename << "_hpp_FileRef; };" << std::endl;
    }
    for(FileList::const_iterator it = _cppFileList.begin(), ite = _cppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_cpp_BuildFile = {isa = PBXBuildFile; fileRef = " << basename << "_cpp_FileRef; };" << std::endl;
    }
    for(FileList::const_iterator it = _zppFileList.begin(), ite = _zppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_zpp_BuildFile = {isa = PBXBuildFile; fileRef = " << basename << "_zpp_FileRef; };" << std::endl;
    }
    for(FileList::const_iterator it = _otherFileList.begin(), ite = _otherFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        z::string ext = z::dir::getExtention(filename);
        if(ext == "re") {
            os() << "        " << basename << "_re_BuildFile = {isa = PBXBuildFile; fileRef = " << basename << "_re_FileRef; };" << std::endl;
        } else if(ext == "y") {
            os() << "        " << basename << "_y_BuildFile = {isa = PBXBuildFile; fileRef = " << basename << "_y_FileRef; };" << std::endl;
        } else {
            throw z::Exception("XcodeGenerator", zfmt(z::Ast::Token(filename, 0, 0, ""), "1-Unknown file type for: %{s}").arg("s", filename));
        }
    }
    for(FileList::const_iterator it = _guiFileList.begin(), ite = _guiFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
            os() << "        " << basename << "_htm_BuildFile = {isa = PBXBuildFile; fileRef = " << basename << "_htm_FileRef; };" << std::endl;
    }
    os() << "/* End PBXBuildFile section */" << std::endl;
    os() << "" << std::endl;

    os() << "/* Begin PBXBuildRule section */" << std::endl;
    os() << "    lemon_BuildRule = {" << std::endl;
    os() << "        isa = PBXBuildRule;" << std::endl;
    os() << "        compilerSpec = com.apple.compilers.proxy.script;" << std::endl;
    os() << "        filePatterns = \"*.y\";" << std::endl;
    os() << "        fileType = pattern.proxy;" << std::endl;
    os() << "        isEditable = 1;" << std::endl;
    os() << "        outputFiles = (" << std::endl;
    os() << "            \"${DERIVED_FILE_DIR}/${INPUT_FILE_BASE}.cpp\"," << std::endl;
    os() << "        );" << std::endl;
    os() << "        script = \"" << _project.zlibPath() << "/lemon.osx o=.cpp d=${DERIVED_FILE_DIR} -q ${INPUT_FILE_PATH}\";" << std::endl;
    os() << "    };" << std::endl;
    os() << "" << std::endl;

    os() << "    re2c_BuildRule = {" << std::endl;
    os() << "        isa = PBXBuildRule;" << std::endl;
    os() << "        compilerSpec = com.apple.compilers.proxy.script;" << std::endl;
    os() << "        filePatterns = \"*.re\";" << std::endl;
    os() << "        fileType = pattern.proxy;" << std::endl;
    os() << "        isEditable = 1;" << std::endl;
    os() << "        outputFiles = (" << std::endl;
    os() << "            \"${DERIVED_FILE_DIR}/${INPUT_FILE_BASE}.cpp\"," << std::endl;
    os() << "        );" << std::endl;
    os() << "        script = \"" << _project.zlibPath() << "/re2c.osx -f -u -c -o ${DERIVED_FILE_DIR}/${INPUT_FILE_BASE}.cpp ${INPUT_FILE_PATH}\";" << std::endl;
    os() << "    };" << std::endl;

    os() << "    zenlang_BuildRule = {" << std::endl;
    os() << "            isa = PBXBuildRule;" << std::endl;
    os() << "            compilerSpec = com.apple.compilers.proxy.script;" << std::endl;
    os() << "            filePatterns = \"*.zpp\";" << std::endl;
    os() << "            fileType = pattern.proxy;" << std::endl;
    os() << "            isEditable = 1;" << std::endl;
    os() << "            outputFiles = (" << std::endl;
    os() << "                \"${PROJECT_DIR}/${INPUT_FILE_BASE}.ipp\"," << std::endl;
    os() << "                \"${PROJECT_DIR}/${INPUT_FILE_BASE}.hpp\"," << std::endl;
    os() << "                \"${PROJECT_DIR}/${INPUT_FILE_BASE}.cpp\"," << std::endl;
    os() << "            );" << std::endl;
    os() << "            script = \"" << _project.zexePath() << " -g -c ${INPUT_FILE_PATH}\";" << std::endl;
    os() << "    };" << std::endl;
    os() << "/* End PBXBuildRule section */" << std::endl;

    os() << "/* Begin PBXFileReference section */" << std::endl;
    os() << "        Sqlite3_OSX_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = \"compiled.mach-o.dylib\"; name = libsqlite3.dylib; path = usr/lib/libsqlite3.dylib; sourceTree = SDKROOT; };" << std::endl;
    os() << "        Sqlite3_IOS_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = \"compiled.mach-o.dylib\"; name = libsqlite3.dylib; path = Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS6.0.sdk/usr/lib/libsqlite3.dylib; sourceTree = DEVELOPER_DIR; };" << std::endl;
    os() << "        WebKit_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = WebKit.framework; path = System/Library/Frameworks/WebKit.framework; sourceTree = SDKROOT; };" << std::endl;
    os() << "        Cocoa_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = Cocoa.framework; path = System/Library/Frameworks/Cocoa.framework; sourceTree = SDKROOT; };" << std::endl;
    os() << "        AppKit_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = AppKit.framework; path = System/Library/Frameworks/AppKit.framework; sourceTree = SDKROOT; };" << std::endl;
    os() << "        CoreData_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = CoreData.framework; path = System/Library/Frameworks/CoreData.framework; sourceTree = SDKROOT; };" << std::endl;
    os() << "        Foundation1_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = Foundation.framework; path = System/Library/Frameworks/Foundation.framework; sourceTree = SDKROOT; };" << std::endl;
    os() << "        UIKit_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = UIKit.framework; path = Library/Frameworks/UIKit.framework; sourceTree = DEVELOPER_DIR; };" << std::endl;
    os() << "        Foundation2_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = Foundation.framework; path = Library/Frameworks/Foundation.framework; sourceTree = DEVELOPER_DIR; };" << std::endl;
    os() << "        CoreGraphics_Framework_FileRef = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = CoreGraphics.framework; path = Library/Frameworks/CoreGraphics.framework; sourceTree = DEVELOPER_DIR; };" << std::endl;
    os() << "        " << _project.name() << "_Univ_Info_FileRef = {isa = PBXFileReference; lastKnownFileType = text.plist.xml; name = \"" << _project.name() << "-IOS-Info.plist\"; path = \"" << _project.name() << "-IOS-Info.plist\"; sourceTree = \"<group>\"; };" << std::endl;
    os() << "        " << _project.name() << "_Univ_Prefix_FileRef = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.h; name = \"" << _project.name() << "-IOS-Prefix.pch\"; path = \"" << _project.name() << "-IOS-Prefix.pch\"; sourceTree = \"<group>\"; };" << std::endl;
    os() << "        " << _project.name() << "_app_FileRef = {isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = " << _project.name() << ".app; sourceTree = BUILT_PRODUCTS_DIR; };" << std::endl;
    os() << "        " << _project.name() << "_Info_FileRef = {isa = PBXFileReference; lastKnownFileType = text.plist.xml; name = \"" << _project.name() << "-OSX-Info.plist\"; path = \"" << _project.name() << "-OSX-Info.plist\"; sourceTree = \"<group>\"; };" << std::endl;
    os() << "        " << _project.name() << "_PCH_FileRef = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.h; name = \"" << _project.name() << "-OSX-Prefix.pch\"; path = \"" << _project.name() << "-OSX-Prefix.pch\"; sourceTree = \"<group>\"; };" << std::endl;
    os() << "        " << _project.name() << "_Univ_app_FileRef = {isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = \"" << _project.name() << "-univ.app\"; sourceTree = BUILT_PRODUCTS_DIR; };" << std::endl;
    os() << "        zenlang_cpp_FileRef = {isa = PBXFileReference; lastKnownFileType = sourcecode.cpp.objcpp; name = zenlang.cpp; path = \"" << _project.zlibPath() << "/zenlang.cpp\"; sourceTree = \"<absolute>\"; };" << std::endl;
    for(FileList::const_iterator it = _hppFileList.begin(), ite = _hppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_hpp_FileRef = {isa = PBXFileReference; lastKnownFileType = sourcecode.hpp.objhpp; name = " << z::dir::getFilename(filename) << "; path = " << filename << "; sourceTree = \"<group>\"; };" << std::endl;
    }
    for(FileList::const_iterator it = _cppFileList.begin(), ite = _cppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_cpp_FileRef = {isa = PBXFileReference; lastKnownFileType = sourcecode.cpp.objcpp; name = " << z::dir::getFilename(filename) << "; path = " << filename << "; sourceTree = \"<group>\"; };" << std::endl;
    }
    for(FileList::const_iterator it = _zppFileList.begin(), ite = _zppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_zpp_FileRef = {isa = PBXFileReference; lastKnownFileType = sourcecode.zpp; name = " << z::dir::getFilename(filename) << "; path = " << filename << "; sourceTree = \"<group>\"; };" << std::endl;
    }
    for(FileList::const_iterator it = _otherFileList.begin(), ite = _otherFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        z::string ext = z::dir::getExtention(filename);
        if(ext == "re") {
            os() << "        " << basename << "_re_FileRef = {isa = PBXFileReference; lastKnownFileType = sourcecode.re; name = " << z::dir::getFilename(filename) << "; path = " << filename << "; sourceTree = \"<group>\"; };" << std::endl;
        } else if(ext == "y") {
            os() << "        " << basename << "_y_FileRef = {isa = PBXFileReference; lastKnownFileType = sourcecode.y; name = " << z::dir::getFilename(filename) << "; path = " << filename << "; sourceTree = \"<group>\"; };" << std::endl;
        } else {
            throw z::Exception("XcodeGenerator", zfmt(z::Ast::Token(filename, 0, 0, ""), "1-Unknown file type for: %{s}").arg("s", filename));
        }
    }
    for(FileList::const_iterator it = _guiFileList.begin(), ite = _guiFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_htm_FileRef = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = text.html; name = " << z::dir::getFilename(filename) << "; path = " << filename << "; sourceTree = \"<group>\"; };" << std::endl;
    }
    os() << "/* End PBXFileReference section */" << std::endl;
    os() << "" << std::endl;
    os() << "/* Begin PBXFrameworksBuildPhase section */" << std::endl;
    os() << "        OSX_Frameworks_BuildPhase = {" << std::endl;
    os() << "			isa = PBXFrameworksBuildPhase;" << std::endl;
    os() << "			buildActionMask = 2147483647;" << std::endl;
    os() << "			files = (" << std::endl;
    os() << "                Sqlite3_OSX_Framework_BuildFile," << std::endl;
    os() << "                WebKit_Framework_BuildFile," << std::endl;
    os() << "                Cocoa_Framework_BuildFile," << std::endl;
    os() << "			);" << std::endl;
    os() << "			runOnlyForDeploymentPostprocessing = 0;" << std::endl;
    os() << "		};" << std::endl;
    os() << "        IOS_Frameworks_BuildPhase = {" << std::endl;
    os() << "			isa = PBXFrameworksBuildPhase;" << std::endl;
    os() << "			buildActionMask = 2147483647;" << std::endl;
    os() << "			files = (" << std::endl;
    os() << "                Sqlite3_IOS_Framework_BuildFile," << std::endl;
    os() << "                UIKit_Framework_BuildFile," << std::endl;
    os() << "                Foundation_Framework_BuildFile," << std::endl;
    os() << "                CoreGraphics_Framework_BuildFile," << std::endl;
    os() << "			);" << std::endl;
    os() << "			runOnlyForDeploymentPostprocessing = 0;" << std::endl;
    os() << "		};" << std::endl;
    os() << "/* End PBXFrameworksBuildPhase section */" << std::endl;
    os() << "" << std::endl;
    os() << "/* Begin PBXGroup section */" << std::endl;
    os() << "        Top_Group = {" << std::endl;
    os() << "			isa = PBXGroup;" << std::endl;
    os() << "			children = (" << std::endl;
    os() << "                " << _project.name() << "_Univ_Prefix_FileRef," << std::endl;
    os() << "                " << _project.name() << "_Univ_Info_FileRef," << std::endl;
    os() << "                " << _project.name() << "_Info_FileRef," << std::endl;
    os() << "                zenlang_cpp_FileRef," << std::endl;
    for(FileList::const_iterator it = _hppFileList.begin(), ite = _hppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_hpp_FileRef," << std::endl;
    }
    for(FileList::const_iterator it = _cppFileList.begin(), ite = _cppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_cpp_FileRef," << std::endl;
    }
    for(FileList::const_iterator it = _zppFileList.begin(), ite = _zppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "        " << basename << "_zpp_FileRef," << std::endl;
    }
    for(FileList::const_iterator it = _otherFileList.begin(), ite = _otherFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        z::string ext = z::dir::getExtention(filename);
        if(ext == "re") {
            os() << "        " << basename << "_re_FileRef," << std::endl;
        } else if(ext == "y") {
            os() << "        " << basename << "_y_FileRef," << std::endl;
        } else {
            throw z::Exception("XcodeGenerator", zfmt(z::Ast::Token(filename, 0, 0, ""), "1-Unknown file type for: %{s}").arg("s", filename));
        }
    }
    for(FileList::const_iterator it = _guiFileList.begin(), ite = _guiFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
            os() << "        " << basename << "_htm_FileRef," << std::endl;
    }
    os() << "                " << _project.name() << "_PCH_FileRef," << std::endl;
    os() << "                Frameworks_Group," << std::endl;
    os() << "                Products_Group," << std::endl;
    os() << "			);" << std::endl;
    os() << "            sourceTree = \"<group>\";" << std::endl;
    os() << "		};" << std::endl;
    os() << "        Products_Group = {" << std::endl;
    os() << "			isa = PBXGroup;" << std::endl;
    os() << "			children = (" << std::endl;
    os() << "                " << _project.name() << "_app_FileRef," << std::endl;
    os() << "                " << _project.name() << "_Univ_app_FileRef," << std::endl;
    os() << "			);" << std::endl;
    os() << "			name = Products;" << std::endl;
    os() << "            sourceTree = \"<group>\";" << std::endl;
    os() << "		};" << std::endl;
    os() << "        Frameworks_Group = {" << std::endl;
    os() << "			isa = PBXGroup;" << std::endl;
    os() << "			children = (" << std::endl;
    os() << "                Sqlite3_OSX_Framework_FileRef," << std::endl;
    os() << "                Sqlite3_IOS_Framework_FileRef," << std::endl;
    os() << "                WebKit_Framework_FileRef," << std::endl;
    os() << "                Cocoa_Framework_FileRef," << std::endl;
    os() << "                UIKit_Framework_FileRef," << std::endl;
    os() << "                Foundation2_Framework_FileRef," << std::endl;
    os() << "                CoreGraphics_Framework_FileRef," << std::endl;
    os() << "                AppKit_Framework_FileRef," << std::endl;
    os() << "                CoreData_Framework_FileRef," << std::endl;
    os() << "                Foundation1_Framework_FileRef," << std::endl;
    os() << "			);" << std::endl;
    os() << "			name = Frameworks;" << std::endl;
    os() << "            sourceTree = \"<group>\";" << std::endl;
    os() << "		};" << std::endl;
    os() << "/* End PBXGroup section */" << std::endl;
    os() << "" << std::endl;
    os() << "/* Begin PBXNativeTarget section */" << std::endl;
    os() << "        " << _project.name() << "_OSX_Target = {" << std::endl;
    os() << "			isa = PBXNativeTarget;" << std::endl;
    os() << "            buildConfigurationList = OSX_Target_ConfigList;" << std::endl;
    os() << "			buildPhases = (" << std::endl;
    os() << "                " << _project.name() << "_OSX_Sources," << std::endl;
    os() << "                OSX_Frameworks_BuildPhase," << std::endl;
    os() << "                " << _project.name() << "_OSX_Resources," << std::endl;
    os() << "			);" << std::endl;
    os() << "			buildRules = (" << std::endl;
    os() << "			    re2c_BuildRule," << std::endl;
    os() << "			    lemon_BuildRule," << std::endl;
    os() << "			    zenlang_BuildRule," << std::endl;
    os() << "			);" << std::endl;
    os() << "			dependencies = (" << std::endl;
    os() << "			);" << std::endl;
    os() << "			name = " << _project.name() << ";" << std::endl;
    os() << "			productName = " << _project.name() << ";" << std::endl;
    os() << "            productReference = " << _project.name() << "_app_FileRef;" << std::endl;
    os() << "            productType = \"com.apple.product-type.application\";" << std::endl;
    os() << "		};" << std::endl;
    os() << "        " << _project.name() << "_IOS_Target = {" << std::endl;
    os() << "			isa = PBXNativeTarget;" << std::endl;
    os() << "            buildConfigurationList = IOS_Target_ConfigList;" << std::endl;
    os() << "			buildPhases = (" << std::endl;
    os() << "                " << _project.name() << "_IOS_Sources," << std::endl;
    os() << "                IOS_Frameworks_BuildPhase," << std::endl;
    os() << "                " << _project.name() << "_IOS_Resources," << std::endl;
    os() << "			);" << std::endl;
    os() << "			buildRules = (" << std::endl;
    os() << "			    re2c_BuildRule," << std::endl;
    os() << "			    lemon_BuildRule," << std::endl;
    os() << "			    zenlang_BuildRule," << std::endl;
    os() << "			);" << std::endl;
    os() << "			dependencies = (" << std::endl;
    os() << "			);" << std::endl;
    os() << "            name = \"" << _project.name() << "-univ\";" << std::endl;
    os() << "            productName = \"" << _project.name() << "-univ\";" << std::endl;
    os() << "            productReference = " << _project.name() << "_Univ_app_FileRef;" << std::endl;
    os() << "            productType = \"com.apple.product-type.application\";" << std::endl;
    os() << "		};" << std::endl;
    os() << "/* End PBXNativeTarget section */" << std::endl;
    os() << "" << std::endl;
    os() << "/* Begin PBXProject section */" << std::endl;
    os() << "        " << _project.name() << "_Project = {" << std::endl;
    os() << "			isa = PBXProject;" << std::endl;
    os() << "			attributes = {" << std::endl;
    os() << "				LastUpgradeCheck = 0450;" << std::endl;
    os() << "                ORGANIZATIONNAME = \"Renji Panicker\";" << std::endl;
    os() << "			};" << std::endl;
    os() << "            buildConfigurationList = Project_ConfigList;" << std::endl;
    os() << "            compatibilityVersion = \"Xcode 3.2\";" << std::endl;
    os() << "			developmentRegion = English;" << std::endl;
    os() << "			hasScannedForEncodings = 0;" << std::endl;
    os() << "			knownRegions = (" << std::endl;
    os() << "				en," << std::endl;
    os() << "			);" << std::endl;
    os() << "            mainGroup = Top_Group;" << std::endl;
    os() << "            productRefGroup = Products_Group;" << std::endl;
    os() << "            projectDirPath = \"\";" << std::endl;
    os() << "            projectRoot = \"\";" << std::endl;
    os() << "			targets = (" << std::endl;
    os() << "                " << _project.name() << "_OSX_Target," << std::endl;
    os() << "                " << _project.name() << "_IOS_Target," << std::endl;
    os() << "			);" << std::endl;
    os() << "		};" << std::endl;
    os() << "/* End PBXProject section */" << std::endl;
    os() << "" << std::endl;
    os() << "/* Begin PBXResourcesBuildPhase section */" << std::endl;
    os() << "        " << _project.name() << "_OSX_Resources = {" << std::endl;
    os() << "			isa = PBXResourcesBuildPhase;" << std::endl;
    os() << "			buildActionMask = 2147483647;" << std::endl;
    os() << "			files = (" << std::endl;
    for(FileList::const_iterator it = _guiFileList.begin(), ite = _guiFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
            os() << "        " << basename << "_htm_BuildFile," << std::endl;
    }
    os() << "			);" << std::endl;
    os() << "			runOnlyForDeploymentPostprocessing = 0;" << std::endl;
    os() << "		};" << std::endl;
    os() << "        " << _project.name() << "_IOS_Resources = {" << std::endl;
    os() << "			isa = PBXResourcesBuildPhase;" << std::endl;
    os() << "			buildActionMask = 2147483647;" << std::endl;
    os() << "			files = (" << std::endl;
    for(FileList::const_iterator it = _guiFileList.begin(), ite = _guiFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
            os() << "        " << basename << "_htm_BuildFile," << std::endl;
    }
    os() << "			);" << std::endl;
    os() << "			runOnlyForDeploymentPostprocessing = 0;" << std::endl;
    os() << "		};" << std::endl;
    os() << "/* End PBXResourcesBuildPhase section */" << std::endl;
    os() << "" << std::endl;
    os() << "/* Begin PBXSourcesBuildPhase section */" << std::endl;
    os() << "        " << _project.name() << "_OSX_Sources = {" << std::endl;
    os() << "			isa = PBXSourcesBuildPhase;" << std::endl;
    os() << "			buildActionMask = 2147483647;" << std::endl;
    os() << "			files = (" << std::endl;
    os() << "                zenlang_cpp_BuildFile," << std::endl;
    for(FileList::const_iterator it = _otherFileList.begin(), ite = _otherFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        z::string ext = z::dir::getExtention(filename);
        if(ext == "re") {
            os() << "                " << basename << "_re_BuildFile," << std::endl;
        } else if(ext == "y") {
            os() << "                " << basename << "_y_BuildFile," << std::endl;
        } else {
            throw z::Exception("XcodeGenerator", zfmt(z::Ast::Token(filename, 0, 0, ""), "1-Unknown file type for: %{s}").arg("s", filename));
        }
    }
    for(FileList::const_iterator it = _zppFileList.begin(), ite = _zppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "                " << basename << "_zpp_BuildFile," << std::endl;
    }
    for(FileList::const_iterator it = _hppFileList.begin(), ite = _hppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "                " << basename << "_hpp_BuildFile," << std::endl;
    }
    for(FileList::const_iterator it = _cppFileList.begin(), ite = _cppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "                " << basename << "_cpp_BuildFile," << std::endl;
    }
    os() << "			);" << std::endl;
    os() << "			runOnlyForDeploymentPostprocessing = 0;" << std::endl;
    os() << "		};" << std::endl;
    os() << "        " << _project.name() << "_IOS_Sources = {" << std::endl;
    os() << "			isa = PBXSourcesBuildPhase;" << std::endl;
    os() << "			buildActionMask = 2147483647;" << std::endl;
    os() << "			files = (" << std::endl;
    os() << "                zenlang_cpp_BuildFile," << std::endl;
    for(FileList::const_iterator it = _otherFileList.begin(), ite = _otherFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        z::string ext = z::dir::getExtention(filename);
        if(ext == "re") {
            os() << "                " << basename << "_re_BuildFile," << std::endl;
        } else if(ext == "y") {
            os() << "                " << basename << "_y_BuildFile," << std::endl;
        } else {
            throw z::Exception("XcodeGenerator", zfmt(z::Ast::Token(filename, 0, 0, ""), "1-Unknown file type for: %{s}").arg("s", filename));
        }
    }
    for(FileList::const_iterator it = _zppFileList.begin(), ite = _zppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "                " << basename << "_zpp_BuildFile," << std::endl;
    }
    for(FileList::const_iterator it = _hppFileList.begin(), ite = _hppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "                " << basename << "_hpp_BuildFile," << std::endl;
    }
    for(FileList::const_iterator it = _cppFileList.begin(), ite = _cppFileList.end(); it != ite; ++it) {
        const z::string& filename = *it;
        z::string basename = z::dir::getBaseName(filename);
        os() << "                " << basename << "_cpp_BuildFile," << std::endl;
    }
    os() << "			);" << std::endl;
    os() << "			runOnlyForDeploymentPostprocessing = 0;" << std::endl;
    os() << "		};" << std::endl;
    os() << "/* End PBXSourcesBuildPhase section */" << std::endl;
    os() << "" << std::endl;
    for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const z::Ast::Config& config = z::ref(it->second);
        generateConfig(config, os);
    }
    os() << "	};" << std::endl;
    os() << "   rootObject = " << _project.name() << "_Project;" << std::endl;
    os() << "}" << std::endl;
    os() << "" << std::endl;

    generatePCHFile();
    generatePListFile();
}

void z::XcodeGenerator::Impl::run() {
    generateProject();
}

z::XcodeGenerator::XcodeGenerator(const z::Ast::Project& project) : _impl(0) {_impl = new Impl(project);}
z::XcodeGenerator::~XcodeGenerator() {delete _impl;}
void z::XcodeGenerator::run() {return z::ref(_impl).run();}
