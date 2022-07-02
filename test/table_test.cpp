#include "zeroerr/table.h"
#include "zeroerr/unittest.h"

#include <iostream>

using namespace zeroerr;

TEST_CASE("table create") {
    Table table("demo table");
    table.set_header({"col1", "col2", "col3"});
    table.add_row({"row1", "@ row2", "row3"});
    table.add_row({"another row1", "row2", "row3"});
    table.add_row({"row1", "row2", "test row3"});
    std::cerr << "table:" << std::endl << table.str() << std::endl;

    Table::Config cfg;
    cfg.show_tb_border = false;
    std::cerr << "remove top bottom border:" << std::endl << table.str(cfg) << std::endl;

    cfg.show_lr_border = false;
    std::cerr << "remove left right border:" << std::endl << table.str(cfg) << std::endl;

    cfg.show_col_split = false;
    std::cerr << "remove column split:" << std::endl << table.str(cfg) << std::endl;

    cfg.show_row_split = false;
    std::cerr << "remove row split:" << std::endl << table.str(cfg) << std::endl;

    cfg.show_header_split = false;
    std::cerr << "remove header split:" << std::endl << table.str(cfg) << std::endl;
}


TEST_CASE("table coverted from vector") {
    Table table("demo table");
    table.set_header({"col1", "col2", "col3"});
    table.add_row("test", 1.25, 'p');
    table.add_rows(std::vector<float>{1.25, 2.56, 231.2}, std::vector<int>{1, 2, 3});
    std::cerr << table.str() << std::endl;
}
