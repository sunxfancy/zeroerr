#pragma once

#include <zeroerr/dbg.h>
#include <zeroerr/internal/config.h>
#include <zeroerr/print.h>

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
    void add_row(std::initializer_list<std::string> _row) { cells.push_back(_row); }

    template <typename T, typename... Args>
    void push_back(std::vector<std::string>& row, T&& t, Args&&... args) {
        _push_back(rank<2>{}, row, std::forward<T>(t));
        push_back(row, std::forward<Args>(args)...);
    }

    template <typename T>
    void push_back(std::vector<std::string>& row, T&& t) {
        _push_back(rank<2>{}, row, std::forward<T>(t));
    }


    ZEROERR_ENABLE_IF(!ZEROERR_IS_CONTAINER)
    _push_back(rank<0>, std::vector<std::string>& row, T&& t) {
        Printer print;
        print.isCompact  = true;
        print.isQuoted   = false;
        print.line_break = "";
        print(std::forward<T>(t));
        row.push_back(print.str());
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_STRING)
    _push_back(rank<2>, std::vector<std::string>& row, T t) {
        row.push_back(std::forward<std::string>(t));
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CONTAINER)
    _push_back(rank<1>, std::vector<std::string>& row, const T& t) {
        for (auto& ele : t) {
            Printer print;
            print.isCompact  = true;
            print.isQuoted   = false;
            print.line_break = "";
            print(ele);
            row.push_back(print.str());
        }
    }

    template <typename... Args>
    void add_row(Args&&... args) {
        std::vector<std::string> row;
        push_back(row, std::forward<Args>(args)...);
        cells.push_back(row);
    }

    template <typename T, typename... Args>
    void add_rows(T&& t, Args&&... args) {
        add_row(std::forward<T>(t));
        add_rows(std::forward<Args>(args)...);
    }

    template <typename T>
    void add_rows(T&& t) {
        add_row(std::forward<T>(t));
    }

protected:
    std::string title;
    unsigned    width = 0, height = 0;  // auto-calculated

    std::vector<unsigned>                 col_width;
    std::vector<std::string>              header, footer;
    std::vector<std::vector<std::string>> cells;
};


}  // namespace  zeroerr
