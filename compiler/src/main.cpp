#include "config.h"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

enum class CompileResultCode {
    Succ,
    ReadSrcFail,
    OpenResFail,
    OpenOutFail,
    UnknownError
};
CompileResultCode compile(const std::filesystem::path& src_path);


int main(int argc, char** argv) {
    auto rc = Config::ParseToInstance(argc, argv);
    if (rc != 0) return -rc;
    const auto& cfg = Config::Current();

    namespace fs = std::filesystem;

    // 加载 src-list
    std::vector<fs::path> src_files;
    {
        std::ifstream in(cfg.src_list_file);
        if (!in) {
            std::cerr << "Failed to open src-list-file: " << cfg.src_list_file<< std::endl;
            return 10;
        }
        std::string line {};
        while (std::getline(in, line)) {
            if (!line.empty()) {
                const auto& path = src_files.emplace_back(line);
                try {
                    if (!fs::is_regular_file(path)) {
                        std::cerr << "Path is not regular file: " << path << std::endl;
                        return 11;
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "filesystem error: " << e.path1()
                        << ": " << e.what() << std::endl;
                    return e.code().value();
                }
                std::cout << "[debug] load file form src-list: " << path << std::endl;
            }
        }
        in.close();
    }
    std::cout << "[debug] load " << src_files.size() << " files form " << cfg.src_list_file << std::endl;

    // 准备 out-dir
    try {
        const auto& p = cfg.out_dir;
        const auto st = fs::status(p);
        if (fs::exists(st)) {
            if (fs::remove_all(p) == 0) {
                std::cerr << "Failed to remove " << p << std::endl;
                return 12;
            }
        }

        if (!fs::create_directories(p)) {
            std::cerr << "Failed to create directory: " << p << std::endl;
            return 13;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "filesystem error: " << e.path1() << ": " << e.what();
        return e.code().value();
    }

    for (const auto& src_path : src_files) {
        if (auto result = compile(src_path); result != CompileResultCode::Succ) {
            return 20 + static_cast<int>(result);
        }
    }

    return 0;
}

struct Hit {
    size_t pos {0};

    explicit Hit(size_t pos) : pos(pos) {}
};

enum class ErcKeywords {
    ErcImport,
    ErcExport    
};

template<ErcKeywords KW>
struct ErcKeyword;

template<>
struct ErcKeyword<ErcKeywords::ErcImport> : Hit {
    static inline const std::regex regex = std::regex(R"re(ERC_IMPORT\s*\(\s*(\w+)\s*,\s*"([^"]*)"\s*\))re");

    std::string name {};
    std::string path {};
    
    explicit ErcKeyword(const std::smatch& smatch)
        : Hit(smatch.position()), name(smatch[1].str()), path(smatch[2].str()) {}
};

template<>
struct ErcKeyword<ErcKeywords::ErcExport> : Hit {
    static inline const std::regex regex = std::regex(R"re(ERC_EXPORT\s*\(\s*(\w+)\s*,\s*"([^"]*)"\s*\))re");

    std::string name {};
    std::string id   {};
    
    explicit ErcKeyword(const std::smatch& smatch)
        : Hit(smatch.position()), name(smatch[1].str()), id(smatch[2].str()) {}
};

using ErcKeywordSet = std::vector<std::variant<
    ErcKeyword<ErcKeywords::ErcImport>,
    ErcKeyword<ErcKeywords::ErcExport>
>>;

template<class... Args>
struct Overloaded : Args... {
    using Args::operator()...;
};
template<class... Args>
Overloaded(Args&&...) -> Overloaded<Args...>;



CompileResultCode compile(const std::filesystem::path& src_path) {
    auto [src, succ] = [&src_path]() -> std::pair<std::string, bool> {
        std::ifstream ifs(src_path);
        if (!ifs.good()) {
            std::cerr << "Failed to open file: " << src_path << std::endl;
            return {"", false};
        }
        std::string src(std::filesystem::file_size(src_path), 0);
        ifs.read(src.data(), src.size());
        ifs.close();

        std::cout << "[debug] read file " << src_path << ", len=" << src.size() << std::endl;

        return {src, true};
    }();
    if (!succ) return CompileResultCode::ReadSrcFail;
    if (src.empty()) return CompileResultCode::Succ;
    
    
    ErcKeywordSet set {};
    auto collect = [&]<ErcKeywords KW>() {
        using Keyword = ErcKeyword<KW>;
        for (auto it = std::sregex_iterator(src.begin(), src.end(), Keyword::regex);
                it != std::sregex_iterator();
                ++it
        ) {
            set.push_back(Keyword(*it));
        }
    };

    collect.template operator()<ErcKeywords::ErcImport>();
    collect.template operator()<ErcKeywords::ErcExport>();

    if (set.empty()) return CompileResultCode::Succ;

    
    std::sort(set.begin(), set.end(), [](const auto& a, const auto& b) {
        constexpr auto getpos = [](auto&& v) { return v.pos; };
        return std::visit(getpos, a) < std::visit(getpos, b);
    });


    /*********************************************************
    // @global
    #pragma once
    #include <array>
    #include <byte>

    namespace erc::resources {
        using B = std::byte;
    }

    // @local: import
    namespace erc {
        namespace resources {
            std::array<std::byte, file_size(%path%)> %name% {
                B(0x??), ...
            };
        }
        const erc::resource %name%(resources::%name%, %path%);
    }

    // @local: export
    // Macro expansion only, no content generation required
    **********************************************************/

    const auto out_file_path = Config::Current().out_dir / std::string{src_path.filename().string()}.append(".erc.cpp");
    std::ofstream ofs(out_file_path);
    if (!ofs.good()) {
        std::cerr << "Failed to open file: " << out_file_path << std::endl;
        return CompileResultCode::OpenOutFail;
    }

    ofs << "/* THIS FILE IS AUTO GEN BY EASY-RESOURCE-COMPILER, DO NOT EDIT IT */" << std::endl;

    if (!set.empty()) {
        ofs << "#include <array>\n"
            << "#include <cstddef>\n"
            << "#include \"erc/resource.h\"\n"
            << "\n"
            << "namespace erc::resources {\n"
            << "    using B = std::byte;\n"
            << "}\n"
            << "\n";
    }
    
    for (const auto& item : set) {
        namespace fs = std::filesystem;
        
        const auto result = std::visit(Overloaded{
            [&](const ErcKeyword<ErcKeywords::ErcImport>& kw) {
                std::cout << "[debug] keyword(erc_import): name=" << kw.name << ", path=" << kw.path << std::endl;

                const auto path = Config::Current().asset_root / kw.path;
                const size_t file_size = fs::file_size(path);

                // start
                ofs << "namespace erc {\n"
                    << "    namespace resources {\n"
                    << "        const std::array<std::byte, " << file_size << "> " << kw.name << " {";

                // file content
                std::ifstream ifs(path, std::ios::binary | std::ios::ate);
                if (!ifs.good()) {
                    std::cerr << "Failed to open file: " << path << std::endl;
                    return CompileResultCode::OpenResFail;
                }
                ifs.seekg(0, std::ios::beg);

                std::vector<unsigned char> buf(1024 * 1024 * 64);
                size_t written = 0;

                while (ifs) {
                    ifs.read(reinterpret_cast<char*>(buf.data()), buf.size());
                    const auto got = ifs.gcount();
                    if (got <= 0) break;

                    constexpr size_t PER_LINE = 12;
                    for (size_t i = 0; i < static_cast<size_t>(got); ++i) {
                        const size_t gl_index = written + i;
                        if (gl_index % PER_LINE == 0)
                            ofs << "\n" << std::string(12, ' ');

                        ofs << "B(0x"
                            << std::hex << std::uppercase
                            << std::setw(2) << std::setfill('0')
                            << static_cast<int>(buf[i])
                            << ")";

                        if (gl_index + 1 != file_size) ofs << ", ";
                    }

                    written += static_cast<size_t>(got);
                }
                ofs << "\n";

                // tail
                ofs << "        };\n"
                    << "    }\n"
                    << "    constinit ::erc::resource " << kw.name << "("
                        "\"" << kw.name << "\", "
                        "\"" << kw.path << "\", "
                        "resources::" << kw.name << ");\n"
                    << "}\n"
                    << std::endl;

                return CompileResultCode::Succ;
            },
            [&](const ErcKeyword<ErcKeywords::ErcExport>& kw) {
                std::cout << "[debug] keyword(erc_export): name=" << kw.name << ", id=" << kw.id << std::endl;

                return CompileResultCode::Succ;
            },
            [](auto&& n) {
                std::cerr << "Unknown keyword type: " << typeid(n).name() << std::endl;
                return CompileResultCode::UnknownError;
            }
        }, item);

        if (result != CompileResultCode::Succ) return result;
    }
    return CompileResultCode::Succ;
}
