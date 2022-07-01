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
    std::cerr << "table:" << std::endl << table.str(cfg) << std::endl;

    cfg.show_lr_border = false;
    std::cerr << "table:" << std::endl << table.str(cfg) << std::endl;

    cfg.show_col_split = false;
    std::cerr << "table:" << std::endl << table.str(cfg) << std::endl;

    cfg.show_row_split = false;
    std::cerr << "table:" << std::endl << table.str(cfg) << std::endl;

    cfg.show_header_split = false;
    std::cerr << "table:" << std::endl << table.str(cfg) << std::endl;
}
