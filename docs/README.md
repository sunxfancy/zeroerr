文档构建
================

本项目使用 [vuepress](https://vuepress.vuejs.org/), [doxygen](https://www.doxygen.nl/) 以及 [doxybook2](https://github.com/matusnovak/doxybook2) 构建文档。

### 文档的书写

文档的书写使用 [markdown](https://www.markdownguide.org/) 语法，具体语法参考 [markdown 语法](https://www.markdownguide.org/basic-syntax/) 以及 [doxygen markdown support](https://www.doxygen.nl/manual/markdown.html)。

分中英双语书写，分别放到 `docs/zh` 和 `docs/en` 目录下，文件名保持一致。

### 文档构建依赖

文档必须在linux环境下构建，构建前需要安装 `doxygen` 和 `nodejs`，并使用 `yarn` 安装依赖。

具体可以参考: [nodejs 安装](https://nodejs.org/en/download/package-manager/) 以及 [yarn 安装](https://classic.yarnpkg.com/en/docs/install/#debian-stable)。

Ubuntu下安装的具体方法：

```bash
# 安装 doxygen
sudo apt install doxygen

# 安装 nodejs
sudo apt-get update
sudo apt-get install -y ca-certificates curl gnupg
sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | sudo gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg

NODE_MAJOR=20
echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" | sudo tee /etc/apt/sources.list.d/nodesource.list

sudo apt-get update
sudo apt-get install nodejs -y
```


安装依赖 `vuepress`：

```bash
yarn
```

### 构建文档

首先构建linux版本：
    
```bash
make linux
```

然后构建文档：

```bash
make cmake:docs
```

或者启动开发服务器:

```bash
make cmake:dev
```

