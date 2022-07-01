#pragma once

#include <zeroerr/internal/config.h>
#include <ostream>
#include <string>
#include <vector>

namespace zeroerr {


struct Card {
    Card(std::string title) : title(title) {}

    std::string title;
    unsigned    width, height;

    void show(std::ostream& os, std::string str);
};

class Table {
public:
    Table() {}
    Table(std::string title) : title(title) {}
    Table(unsigned width, unsigned height) : width(width), height(height) {}
    Table(std::string title, unsigned width, unsigned height)
        : title(title), width(width), height(height) {}
    ~Table() {}

    Table& csv(std::string path);
    Table& json(std::string path);

    struct Style {
        Style() {}
        Style(std::initializer_list<std::string> args) : m_args(args) {}
        std::vector<std::string> m_args;

        operator bool() const { return !m_args.empty(); }
    };
    static void  registerStyle(std::string name, Style style);
    static Style getStyle(std::string name);

    struct Config {
        bool show_tb_border;
        bool show_lr_border;
        bool show_header_split;
        bool show_col_split;
        bool show_row_split;
        bool show_footer_split;
        Config()
            : show_tb_border(true),
              show_lr_border(true),
              show_header_split(true),
              show_col_split(true),
              show_row_split(true),
              show_footer_split(true) {}
    };

    std::string str(Config config = Config(), Style style = Table::getStyle("square_double_head"));

    void set_header(std::vector<std::string> _header) { header = _header; }
    void add_row(std::vector<std::string> row) { cells.push_back(row); }

protected:
    std::string title;
    unsigned    width = 0, height = 0;  // auto-calculated


    std::vector<unsigned>                 col_width;
    std::vector<std::string>              header, footer;
    std::vector<std::vector<std::string>> cells;
};


}  // namespace  zeroerr
