#pragma once

#include <zeroerr/dbg.h>
#include <zeroerr/internal/config.h>
#include <zeroerr/print.h>

#include <ostream>
#include <string>
#include <vector>

namespace zeroerr {

/**
 * @brief Card defines a display range in the console.
 */
struct Card {
    Card() : title(), width(0), height(0) {}
    Card(std::string title) : title(title), width(0), height(0) {}
    Card(unsigned width, unsigned height) : title(), width(width), height(height) {}
    Card(std::string title, unsigned width, unsigned height)
        : title(title), width(width), height(height) {}
    std::string title;
    unsigned    width, height;

    void show(std::ostream& os, std::string str);
};

/**
 * @brief Table is used to generate a table with configurable style.
 */
class Table : public Card {
public:
    Table() : Card() {}
    Table(std::string title) : Card(title) {}
    Table(unsigned width, unsigned height) : Card(width, height) {}
    Table(std::string title, unsigned width, unsigned height) : Card(title, width, height) {}
    ~Table() {}

    /**
     * @brief Style is used to define the border style of the table.
     */
    struct Style {
        Style() {}
        Style(std::initializer_list<std::string> args) : m_args(args) {}
        std::vector<std::string> m_args;

        operator bool() const { return !m_args.empty(); }
    };

    static void registerStyle(std::string name, Style style);

    /**
     * @brief getStyle can load predefined style from the StyleManager.
     * @param name is the name of the style.
     * Available styles:
     *  "ascii"
     *  "ascii2"
     *  "ascii_double_head"
     *  "square"
     *  "square_double_head"
     *  "simple"
     *  "simple_head"
     *  "simple_heavy"
     *  "horizontal"
     *  "rounded"
     *  "heavy"
     *  "heavy_edge"
     *  "heavy_head"
     *  "double"
     *  "double_edge"
     *  "minimal"
     *  "minimal_heavy_head"
     *  "minimal_double_hea
     */
    static Style getStyle(std::string name);

    /**
     * @brief Config is used to configure the table style how it is displayed.
     */
    struct Config {
        bool show_tb_border;     // show top and bottom border
        bool show_lr_border;     // show left and right border
        bool show_header_split;  // show header split
        bool show_col_split;     // show column split
        bool show_row_split;     // show row split
        bool show_footer_split;  // show footer split
        Config()
            : show_tb_border(true),
              show_lr_border(true),
              show_header_split(true),
              show_col_split(true),
              show_row_split(true),
              show_footer_split(true) {}
    };

    /**
     * @brief str is used to generate the table string.
     * @param config decides how the table is displayed.
     * @param style decides the border style of the table.
     */
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

    std::vector<unsigned>                 col_width;
    std::vector<std::string>              header, footer;
    std::vector<std::vector<std::string>> cells;
};


}  // namespace  zeroerr
