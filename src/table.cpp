#include "zeroerr/table.h"
#include "zeroerr/log.h"
#include "zeroerr/assert.h"

#include <algorithm>
#include <map>
#include <sstream>

namespace zeroerr {

// clang-format off

static Table::Style ASCII{
"+","-","-","+",
"|"," ","|","|",
"|","-","+","|",
"|"," ","|","|",
"|","-","+","|",
"|","-","+","|",
"|"," ","|","|",
"+","-","-","+",
};

static Table::Style ASCII2{
"+","-","+","+",
"|"," ","|","|",
"+","-","+","+",
"|"," ","|","|",
"+","-","+","+",
"+","-","+","+",
"|"," ","|","|",
"+","-","+","+",
};

static Table::Style ASCII_DOUBLE_HEAD{
"+","-","+","+",
"|"," ","|","|",
"+","=","+","+",
"|"," ","|","|",
"+","-","+","+",
"+","-","+","+",
"|"," ","|","|",
"+","-","+","+",
};

static Table::Style SQUARE{
"┌","─","┬","┐",
"│"," ","│","│",
"├","─","┼","┤",
"│"," ","│","│",
"├","─","┼","┤",
"├","─","┼","┤",
"│"," ","│","│",
"└","─","┴","┘",
};

static Table::Style SQUARE_DOUBLE_HEAD{
"┌","─","┬","┐",
"│"," ","│","│",
"╞","═","╪","╡",
"│"," ","│","│",
"├","─","┼","┤",
"├","─","┼","┤",
"│"," ","│","│",
"└","─","┴","┘",
};

static Table::Style MINIMAL{
" "," ","╷"," ",
" "," ","│"," ",
"╶","─","┼","╴",
" "," ","│"," ",
"╶","─","┼","╴",
"╶","─","┼","╴",
" "," ","│"," ",
" "," ","╵"," ",
};


static Table::Style MINIMAL_HEAVY_HEAD{
" "," ","╷"," ",
" "," ","│"," ",
"╺","━","┿","╸",
" "," ","│"," ",
"╶","─","┼","╴",
"╶","─","┼","╴",
" "," ","│"," ",
" "," ","╵"," ",
};

static Table::Style MINIMAL_DOUBLE_HEAD{
" "," ","╷"," ",
" "," ","│"," ",
" ","═","╪"," ",
" "," ","│"," ",
" ","─","┼"," ",
" ","─","┼"," ",
" "," ","│"," ",
" "," ","╵"," ",
};


static Table::Style SIMPLE{
" "," "," "," ",
" "," "," "," ",
" ","─","─"," ",
" "," "," "," ",
" "," "," "," ",
" ","─","─"," ",
" "," "," "," ",
" "," "," "," ",
};

static Table::Style SIMPLE_HEAD{
" "," "," "," ",
" "," "," "," ",
" ","─","─"," ",
" "," "," "," ",
" "," "," "," ",
" "," "," "," ",
" "," "," "," ",
" "," "," "," ",
};


static Table::Style SIMPLE_HEAVY{
" "," "," "," ",
" "," "," "," ",
" ","━","━"," ",
" "," "," "," ",
" "," "," "," ",
" ","━","━"," ",
" "," "," "," ",
" "," "," "," ",
};


static Table::Style HORIZONTALS{
" ","─","─"," ",
" "," "," "," ",
" ","─","─"," ",
" "," "," "," ",
" ","─","─"," ",
" ","─","─"," ",
" "," "," "," ",
" ","─","─"," ",
};

static Table::Style ROUNDED{
"╭","─","┬","╮",
"│"," ","│","│",
"├","─","┼","┤",
"│"," ","│","│",
"├","─","┼","┤",
"├","─","┼","┤",
"│"," ","│","│",
"╰","─","┴","╯",
};

static Table::Style HEAVY{
"┏","━","┳","┓",
"┃"," ","┃","┃",
"┣","━","╋","┫",
"┃"," ","┃","┃",
"┣","━","╋","┫",
"┣","━","╋","┫",
"┃"," ","┃","┃",
"┗","━","┻","┛",
};

static Table::Style HEAVY_EDGE{
"┏","━","┯","┓",
"┃"," ","│","┃",
"┠","─","┼","┨",
"┃"," ","│","┃",
"┠","─","┼","┨",
"┠","─","┼","┨",
"┃"," ","│","┃",
"┗","━","┷","┛",
};

static Table::Style HEAVY_HEAD{
"┏","━","┳","┓",
"┃"," ","┃","┃",
"┡","━","╇","┩",
"│"," ","│","│",
"├","─","┼","┤",
"├","─","┼","┤",
"│"," ","│","│",
"└","─","┴","┘",
};

static Table::Style DOUBLE{
"╔","═","╦","╗",
"║"," ","║","║",
"╠","═","╬","╣",
"║"," ","║","║",
"╠","═","╬","╣",
"╠","═","╬","╣",
"║"," ","║","║",
"╚","═","╩","╝",
};

static Table::Style DOUBLE_EDGE{
"╔","═","╤","╗",
"║"," ","│","║",
"╟","─","┼","╢",
"║"," ","│","║",
"╟","─","┼","╢",
"╟","─","┼","╢",
"║"," ","│","║",
"╚","═","╧","╝",
};

// clang-format on

struct StyleManager {
    std::map<std::string, Table::Style> styles;

    static StyleManager& instance() {
        static StyleManager instance;
        instance.styles["ascii"]               = ASCII;
        instance.styles["ascii2"]              = ASCII2;
        instance.styles["ascii_double_head"]   = ASCII_DOUBLE_HEAD;
        instance.styles["square"]              = SQUARE;
        instance.styles["square_double_head"]  = SQUARE_DOUBLE_HEAD;
        instance.styles["simple"]              = SIMPLE;
        instance.styles["simple_head"]         = SIMPLE_HEAD;
        instance.styles["simple_heavy"]        = SIMPLE_HEAVY;
        instance.styles["horizontal"]          = HORIZONTALS;
        instance.styles["rounded"]             = ROUNDED;
        instance.styles["heavy"]               = HEAVY;
        instance.styles["heavy_edge"]          = HEAVY_EDGE;
        instance.styles["heavy_head"]          = HEAVY_HEAD;
        instance.styles["double"]              = DOUBLE;
        instance.styles["double_edge"]         = DOUBLE_EDGE;
        instance.styles["minimal"]             = MINIMAL;
        instance.styles["minimal_heavy_head"]  = MINIMAL_HEAVY_HEAD;
        instance.styles["minimal_double_head"] = MINIMAL_DOUBLE_HEAD;
        return instance;
    }
};

void Table::registerStyle(std::string name, Table::Style style) {
    StyleManager::instance().styles[name] = style;
}
Table::Style Table::getStyle(std::string name) { return StyleManager::instance().styles[name]; }

#define left  (c.show_lr_border ? s.m_args[p] : "")
#define space s.m_args[p + 1]
#define bar   (c.show_col_split ? s.m_args[p + 2] : s.m_args[p + 1])
#define right (c.show_lr_border ? s.m_args[p + 3] : "")

#define last (i == header.size() - 1)
#define for_row          \
    if (!skip_lb)        \
        ss << std::endl; \
    else                 \
        skip_lb = false; \
    ss << left;          \
    for (size_t i = 0; i < header.size(); ++i)


inline std::string _rept(unsigned k, std::string j, Table::Style&) {
    std::stringstream ss;
    for (unsigned i = 0; i < k; i++) {
        ss << j;
    }
    return ss.str();
}

#define rep(k, t) _rept(k, t, s)
#define remain(k) (col_width[i] - k.size())

std::string Table::str(Config c, Table::Style s) {
    std::stringstream ss;

    for (auto& row : cells) {
        REQUIRE(row.size() == header.size());
    }

    if (col_width.size() == 0) {
        for (size_t i = 0; i < header.size(); ++i) {
            unsigned max_width = 0;
            for (auto& row : cells) {
                max_width = std::max<unsigned>(row[i].size(), max_width);
            }
            max_width = std::max<unsigned>(max_width, header[i].size());
            col_width.push_back(max_width);
        }
    }

    int  p;
    bool skip_lb = true;
    // Header
    if (c.show_tb_border) {
        p = 0;
        for_row { ss << rep(col_width[i] + 2, space) << (last ? right : bar); }
    }

    p = 4;
    for_row {
        REQUIRE(col_width[i] >= header[i].size());
        ss << space << rep(remain(header[i]), space) << header[i] << space << (last ? right : bar);
    }

    if (c.show_header_split) {
        p = 8;
        for_row { ss << rep(col_width[i] + 2, space) << (last ? right : bar); }
    }

    p = 12;
    // Body
    bool first = true;
    for (auto& row : cells) {
        if (!first && c.show_row_split) {
            p += 4;
            for_row { ss << rep(col_width[i] + 2, space) << (last ? right : bar); }
            p -= 4;
        } else
            first = false;
        for_row {
            REQUIRE(col_width[i] >= row[i].size());
            ss << space << rep(remain(row[i]), space) << row[i] << space << (last ? right : bar);
        }
    }

    if (footer.size() != 0) {
    }

    // Bottom
    if (c.show_tb_border) {
        p = 28;
        for_row { ss << rep(col_width[i] + 2, space) << (last ? right : bar); }
    }
    return ss.str();
}


}  // namespace zeroerr
