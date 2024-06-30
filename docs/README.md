文档构建
================

本项目使用 [doxygen](https://www.doxygen.nl/) 以及 [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) 构建文档。

### 文档的书写

文档的书写使用 [markdown](https://www.markdownguide.org/) 语法，具体语法参考 [markdown 语法](https://www.markdownguide.org/basic-syntax/) 以及 [doxygen markdown support](https://www.doxygen.nl/manual/markdown.html)。

分中英双语书写，分别放到 `docs/pages/zh` 和 `docs/pages/en` 目录下，文件名保持一致。

### 文档构建依赖

构建前需要安装 `doxygen`, 确保其能被 cmake 搜索到

### 构建文档

直接运行构建文档指令：

```bash
make doc
```

生成的文档会出现在 `build-linux-doc/doc_doxygen` 目录下
