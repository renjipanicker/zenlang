#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/MsvcGenerator.hpp"
#include "base/compiler.hpp"

class z::MsvcGenerator::Impl : public z::Generator::Impl {
public:
    inline Impl(const z::Ast::Project& project) : Generator::Impl(project) {}
public:
    void run();
private:
    inline void generateConfig(z::ofile& os, const z::Ast::Config& config);
    inline void writeLibFile(const z::Ast::Project& project, z::ofile& os, const z::string& filename);
    inline void generateGui();
};

inline z::string nfn(const z::string& filename) {
    z::string fn(filename);
    fn.replace("/", "\\");
    return fn;
}

inline void z::MsvcGenerator::Impl::generateGui() {
    z::ofile os("gui.rc");
    for(FileList::const_iterator it = _guiFileList.begin(), ite = _guiFileList.end(); it != ite; ++it) {
        const z::string& f = *it;
        os() << z::dir::getFilename(f) << " HTML DISCARDABLE \"" << f << "\"" << std::endl;
    }
}

inline void z::MsvcGenerator::Impl::generateConfig(z::ofile& os, const z::Ast::Config& config) {
    z::Compiler compiler(_project, config);
    compiler.compile();

    if(config.abstract()) {
        return;
    }

    os() << "        <Configuration" << std::endl;
    os() << "            Name=\"" << config.name() << "|Win32\"" << std::endl;
    os() << "            OutputDirectory=\"$(SolutionDir)$(ConfigurationName)\"" << std::endl;
    os() << "            IntermediateDirectory=\"$(ConfigurationName)\"" << std::endl;
    switch(config.buildMode()) {
    case z::Ast::Config::BuildMode::Executable:
        os() << "            ConfigurationType=\"1\"" << std::endl;
        break;
    case z::Ast::Config::BuildMode::Shared:
        os() << "            ConfigurationType=\"2\"" << std::endl;
        break;
    case z::Ast::Config::BuildMode::Static:
        os() << "            ConfigurationType=\"4\"" << std::endl;
        break;
    }
    os() << "            CharacterSet=\"1\"" << std::endl;
    os() << "            >" << std::endl;
    os() << "            <Tool" << std::endl;
    os() << "                Name=\"VCCLCompilerTool\"" << std::endl;
    os() << "                AdditionalIncludeDirectories=\"";

    os() << nfn(_project.zlibPath()) << ";";
    os() << nfn(_project.zlibPath()) << "\\utils\\sqlite3\\;";
    for(z::stringlist::const_iterator it = config.includePathList().begin(); it != config.includePathList().end(); ++it) {
        const z::string& p = *it;
        os() << nfn(p) << ";";
    }
    os() << "\"" << std::endl;

    z::string defs = "WIN32;";
    defs += ((config.debug())?"_DEBUG;DEBUG;":"_NDEBUG;");
    defs += ((config.gui())?"_WINDOWS;GUI;":"_CONSOLE;");
    defs += ((config.test())?"UNIT_TEST;":"");
    defs += ((config.buildMode() == z::Ast::Config::BuildMode::Executable)?"Z_EXE;":"");

    os() << "                PreprocessorDefinitions=\"" << defs << "\"" << std::endl;

    if(config.debug()) {
        os() << "                Optimization=\"0\"" << std::endl;
        os() << "                MinimalRebuild=\"true\"" << std::endl;
        os() << "                BasicRuntimeChecks=\"3\"" << std::endl;
        os() << "                RuntimeLibrary=\"3\"" << std::endl;
        os() << "                DebugInformationFormat=\"4\"" << std::endl;
    } else {
        os() << "                Optimization=\"2\"" << std::endl;
        os() << "                EnableIntrinsicFunctions=\"true\"" << std::endl;
        os() << "                RuntimeLibrary=\"2\"" << std::endl;
        os() << "                DebugInformationFormat=\"3\"" << std::endl;
    }

    os() << "                UsePrecompiledHeader=\"2\"" << std::endl;
    os() << "                PrecompiledHeaderThrough=\"" << _pch << "\"" << std::endl;
    os() << "                WarningLevel=\"4\"" << std::endl;
    os() << "            />" << std::endl;

    switch(config.buildMode()) {
    case z::Ast::Config::BuildMode::Executable:
        // fall-thru
    case z::Ast::Config::BuildMode::Shared:
        os() << "            <Tool" << std::endl;
        os() << "                Name=\"VCLinkerTool\"" << std::endl;
        os() << "                AdditionalDependencies=\"ws2_32.lib shell32.lib ";
        if(config.gui()) {
            os() << "comctl32.lib ";
        }
        os() << "\"" << std::endl;
        if(config.debug()) {
            os() << "                LinkIncremental=\"2\"" << std::endl;
            os() << "                GenerateDebugInformation=\"true\"" << std::endl;
        } else {
            os() << "                LinkIncremental=\"0\"" << std::endl;
            os() << "                LinkTimeCodeGeneration=\"1\"" << std::endl;
            os() << "                GenerateDebugInformation=\"false\"" << std::endl;
        }
        if(config.gui()) {
            os() << "                SubSystem=\"2\"" << std::endl;
        } else {
            os() << "                SubSystem=\"1\"" << std::endl;
        }
        os() << "                TargetMachine=\"1\"" << std::endl;
        //os() << "                OutputFile=\"$(OutDir)\\$(ProjectName)" << config.getSuffix() << _project.getExt(config) << "\"" << std::endl;
        os() << "            />" << std::endl;
        break;
    case z::Ast::Config::BuildMode::Static:
        os() << "            <Tool" << std::endl;
        os() << "                Name=\"VCLibrarianTool\"" << std::endl;
        //os() << "                OutputFile=\"$(OutDir)\\$(ProjectName)" << config.getSuffix() << _project.getExt(config) << "\"" << std::endl;
        os() << "            />" << std::endl;
        break;
    }
    os() << "        </Configuration>" << std::endl;
}

inline void z::MsvcGenerator::Impl::writeLibFile(const z::Ast::Project& project, z::ofile& os, const z::string& filename) {
    os() << "            <File RelativePath=\"" << nfn(filename) << "\" >" << std::endl;
    for(z::Ast::Project::ConfigList::const_iterator it = project.configList().begin(); it != project.configList().end(); ++it) {
        const z::Ast::Config& config = z::ref(it->second);
        if(config.abstract())
            continue;
        os() << "               <FileConfiguration Name=\"" << config.name() << "|Win32\">" << std::endl;
        os() << "                       <Tool Name=\"VCCLCompilerTool\"" << std::endl;
        os() << "                           WarningLevel=\"4\"" << std::endl;
        os() << "                           UsePrecompiledHeader=\"0\"" << std::endl;
        os() << "                           PrecompiledHeaderThrough=\"" + _pch + "\"" << std::endl;
        os() << "                       />" << std::endl;
        os() << "               </FileConfiguration>" << std::endl;
    }
    os() << "            </File>" << std::endl;
}

void z::MsvcGenerator::Impl::run() {
    z::ofile os(_project.name() + ".vcproj");

    os() << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << std::endl;
    os() << "<VisualStudioProject" << std::endl;
    os() << "    ProjectType=\"Visual C++\"" << std::endl;
    os() << "    Version=\"9.00\"" << std::endl;
    os() << "    Name=\"" << _project.name() << "\"" << std::endl;
    os() << "    ProjectGUID=\"{B25BA5CC-6C1D-4C4F-A03B-8AC9BE4C9CE9}\"" << std::endl;
    os() << "    RootNamespace=\"" << _project.name() << "\"" << std::endl;
    os() << "    Keyword=\"Win32Proj\"" << std::endl;
    os() << "    TargetFrameworkVersion=\"196613\"" << std::endl;
    os() << "    >" << std::endl;

    os() << "    <Platforms>" << std::endl;
    os() << "        <Platform Name=\"Win32\"/>" << std::endl;
    os() << "    </Platforms>" << std::endl;

    os() << "    <Configurations>" << std::endl;
    for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(), ite = _project.configList().end(); it != ite; ++it) {
        const z::Ast::Config& config = z::ref(it->second);
        generateConfig(os, config);
    }

    os() << "    </Configurations>" << std::endl;

    // All files
    os() << "    <Files>" << std::endl;
    // All header files
    os() << "        <Filter" << std::endl;
    os() << "            Name=\"Header Files\"" << std::endl;
    os() << "            Filter=\"h;hpp;hxx;hm;inl;inc;xsd\"" << std::endl;
    os() << "            UniqueIdentifier=\"{93995380-89BD-4b04-88EB-625FBE52EBFB}\"" << std::endl;
    os() << "            >" << std::endl;

    // PCH header file
    os() << "            <File RelativePath=\"" + _pch + "\" >" << std::endl;
    os() << "            </File>" << std::endl;

    // All .hpp files
    for(FileList::const_iterator it = _hppFileList.begin(); it != _hppFileList.end(); ++it) {
        const z::string& f = *it;
        os() << "            <File RelativePath=\"" << nfn(f) << "\" >" << std::endl;
        os() << "            </File>" << std::endl;
    }
    os() << "        </Filter>" << std::endl;

    // All zen source files
    os() << "        <Filter" << std::endl;
    os() << "            Name=\"Zen Source Files\"" << std::endl;
    os() << "            Filter=\"cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx\"" << std::endl;
    os() << "            UniqueIdentifier=\"{4FC737F1-C7A5-4376-A066-2A32D752A2FF}\"" << std::endl;
    os() << "            >" << std::endl;

    writeLibFile(_project, os, _project.zlibPath() + "/zenlang.cpp");
    writeLibFile(_project, os, _project.zlibPath() + "/utils/sqlite3/sqlite3.c");
    writeLibFile(_project, os, _project.zlibPath() + "/utils/base64.cpp");
    writeLibFile(_project, os, _project.zlibPath() + "/utils/cJSON.c");

    //PCH source file
    os() << "            <File RelativePath=\"stdafx.cpp\" >" << std::endl;
    for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const z::Ast::Config& cfg = z::ref(it->second);
        if(cfg.abstract())
            continue;
        os() << "                <FileConfiguration Name=\"" << cfg.name() << "|Win32\" >" << std::endl;
        os() << "                    <Tool Name=\"VCCLCompilerTool\" UsePrecompiledHeader=\"1\" />" << std::endl;
        os() << "                </FileConfiguration>" << std::endl;
    }
    os() << "            </File>" << std::endl;
    os() << "        </Filter>" << std::endl;

    if(_cppFileList.size() > 0) {
        // All source files
        os() << "        <Filter" << std::endl;
        os() << "            Name=\"Source Files\"" << std::endl;
        os() << "            Filter=\"cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx\"" << std::endl;
        os() << "            UniqueIdentifier=\"{4FC737F1-C7A5-4376-A066-2A32D752A2FF}\"" << std::endl;
        os() << "            >" << std::endl;

        // All .cpp files
        for(FileList::const_iterator it = _cppFileList.begin(); it != _cppFileList.end(); ++it) {
            const z::string& f = *it;
            os() << "            <File RelativePath=\"" << nfn(f) << "\" />" << std::endl;
        }
        os() << "        </Filter>" << std::endl;
    }

    // all .zpp source files
    // TODO: vcbuild compiles custom files in reverse order, bottom to top.
    // so we must use to a stack to add the list of zpp files in reverse order.
    z::string zppflags = " -c";
    if(_pch != "zenlang.hpp") {
        zppflags += (" -ph \"" + _pch + "\"");
    }

    if(_pchfile != "zenlang.cpp") {
        zppflags += (" -pf \"" + _pchfile + "\"");
    }

    os() << "        <Filter Name=\"Zen Files\" Filter=\"zpp\" ParseFiles=\"false\" >" << std::endl;
    for(FileList::const_iterator it = _zppFileList.begin(); it != _zppFileList.end(); ++it) {
        const z::string& f = *it;
        os() << "            <File RelativePath=\"" << nfn(f) << "\" >" << std::endl;
        for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
            const z::Ast::Config& cfg = z::ref(it->second);
            if(cfg.abstract())
                continue;
            os() << "                <FileConfiguration Name=\"" << cfg.name() << "|Win32\" >" << std::endl;
            os() << "                    <Tool" << std::endl;
            os() << "                        Name=\"VCCustomBuildTool\"" << std::endl;
            os() << "                        Description=\"Compiling $(InputPath)\"" << std::endl;
            os() << "                        CommandLine='" << _project.zexePath() << zppflags << " \"$(InputPath)\"'" << std::endl;
            os() << "                        Outputs=\" $(InputName).hpp; $(InputName).cpp;\"" << std::endl;

#if 0
            if(unit.getDepList().size() > 0) {
                os() << "                        AdditionalDependencies=\"";
                for(z::stringlist::iterator it(unit.getDepList()); !it.end(); ++it) {
                    const z::string& dep = *it;
                    z::fileinfo fi(dep);
                    os() << _project.getAbsoluteDirPath(gcv(cfg, _project.getBldDir())) << "\\" << fi.getBaseName() << ".cpp;";
                }
                os() << "\"" << std::endl;
            }
#endif
            os() << "                    />" << std::endl;
            os() << "                </FileConfiguration>" << std::endl;
        }
        os() << "            </File>" << std::endl;
    }
    os() << "        </Filter>" << std::endl;

    if(_otherFileList.size() > 0) {
        os() << "        <Filter Name=\"Other Files\" Filter=\"y;re;\" ParseFiles=\"false\" >" << std::endl;
        for(FileList::const_iterator it = _otherFileList.begin(), ite = _otherFileList.end(); it != ite; ++it) {
            const z::string& f = *it;
            os() << "            <File RelativePath=\"" << nfn(f) << "\" >" << std::endl;
            for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
                const z::Ast::Config& cfg = z::ref(it->second);
                if(cfg.abstract())
                    continue;
                os() << "                <FileConfiguration Name=\"" << cfg.name() << "|Win32\" >" << std::endl;
                os() << "                    <Tool" << std::endl;
                os() << "                        Name=\"VCCustomBuildTool\"" << std::endl;
                os() << "                        Description=\"Compiling $(InputPath)\"" << std::endl;
                if(z::dir::getExtention(f) == "y") {
                    os() << "                        CommandLine='" << nfn(_project.zlibPath()) << "\\lemon.exe o=.cpp -q \"$(InputPath)\"'" << std::endl;
                } else if(z::dir::getExtention(f) == "re") {
                    os() << "                        CommandLine='" << nfn(_project.zlibPath()) << "\\re2c.exe -f -u -c -i -o $(InputName).cpp \"$(InputPath)\"'" << std::endl;
                } else {
                    assert(false);
                }
                os() << "                        Outputs=\" $(InputName).cpp;\"" << std::endl;
                os() << "                    />" << std::endl;
                os() << "                </FileConfiguration>" << std::endl;
            }
            os() << "            </File>" << std::endl;
        }
        os() << "        </Filter>" << std::endl;
    }

#if 0
    if(false) {
        // the project file itself
        // TODO: we need the name of the original .zproj file.
        os() << "        <Filter Name=\"Zen Project File\" Filter=\"zproj\" ParseFiles=\"false\" >" << std::endl;
        os() << "            <File RelativePath=\"" << nfn(filename) << "\" >" << std::endl;
        for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
            const z::Ast::Config& cfg = z::ref(it->second);
            os() << "                <FileConfiguration Name=\"" << cfg.name() << "|Win32\" >" << std::endl;
            os() << "                    <Tool" << std::endl;
            os() << "                        Name=\"VCCustomBuildTool\"" << std::endl;
            os() << "                        Description=\"Compiling $(InputPath)\"" << std::endl;
            os() << "                        CommandLine=\"" << _project.zexePath() << " -ip $(InputPath)\"" << std::endl;
            os() << "                        Outputs=\" $(InputName).vcproj;\"" << std::endl;
            os() << "                    />" << std::endl;
            os() << "                </FileConfiguration>" << std::endl;
        }
        os() << "            </File>" << std::endl;
        os() << "        </Filter>" << std::endl;
    }
#endif

    os() << "        <Filter Name=\"Generated Files\" >" << std::endl;

    // all files generated from .zpp files
    for(FileList::const_iterator it = _zppFileList.begin(); it != _zppFileList.end(); ++it) {
        const z::string& f = *it;
        const z::string basename = z::dir::getBaseName(f);

        // The following is a hack.
        // The .ipp and .hpp files are actually config-specific.
        // TODO: Figure out how to add them to project.
        os() << "            <File RelativePath=\".\\" << basename << ".ipp" << "\" />" << std::endl;
        os() << "            <File RelativePath=\".\\" << basename << ".hpp" << "\" />" << std::endl;
        writeLibFile(_project, os, basename + ".cpp");
    }

    for(FileList::const_iterator it = _otherFileList.begin(); it != _otherFileList.end(); ++it) {
        const z::string& f = *it;
        const z::string basename = z::dir::getBaseName(f);
        os() << "            <File RelativePath=\".\\" << basename << ".cpp" << "\" />" << std::endl;
    }

    if(_guiFileList.size() > 0) {
        generateGui();
        os() << "            <File RelativePath=\".\\gui.rc" << "\" />" << std::endl;
    }
    os() << "        </Filter>" << std::endl;

    os() << "    </Files>" << std::endl;
    os() << "</VisualStudioProject>" << std::endl;

    z::ofile pch("stdafx.cpp");
    pch() << "#include \"" << _pch << "\"" << std::endl;
}

z::MsvcGenerator::MsvcGenerator(const z::Ast::Project& project) : _impl(0) {_impl = new Impl(project);}
z::MsvcGenerator::~MsvcGenerator() {delete _impl;}
void z::MsvcGenerator::run() {return z::ref(_impl).run();}
