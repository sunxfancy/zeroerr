# CI 与发布流程

日常修改通过 PR 进入 `dev`；`master` 保存已提升、经过测试的发布候选版本。
不要直接向 `master` 提交修复，也不要用 `master` 覆盖 `dev`：两边有独立提交时，
应合并并保留双方历史。

## 仓库设置（管理员）

本次工作流合入 `dev` 后：

1. 在 Settings → General 中将默认分支设为 `dev`，让新 PR 默认选择 `dev`。
2. 为 `dev` 和 `master` 设置分支保护，要求通过 PR 合并，并将 `PR target` 和
   `CI result` 设为必需检查；首次运行后再选择这些检查。要求分支保持最新、
   禁止强推和删除；如果管理员也必须遵守，关闭绕过规则的权限。
3. 将 `dev` → `master` 的提升 PR 合并，使发布工作流也进入 `master`。
   提升使用 merge commit，保留分支间的祖先关系。

工作流文件本身无法修改默认分支或分支保护。没有必需检查时，失败的 PR 策略
只能提示，无法阻止合并。`PR target` 拒绝所有目标不是 `dev` 的普通 PR；
唯一例外是本仓库的 `dev` → `master` 发布提升。

## 开发 CI

`ci.yml` 在推送到 `dev`、向 `dev`/`master` 提交 PR 及手动触发时运行，
也可以由 `release.yml` 传入不可变的提交 SHA 复用。检查包括：

- Linux、Windows、macOS 上的 C++17 构建、示例及测试；
- GCC/Clang 的 C++11、C++14、C++20 兼容性；
- 独立头文件、Clang 警告检查、生成的单头文件一致性；
- 域和序列化的 ASan/UBSan 检查及内存泄漏检测；
- Clang 真实逐测试覆盖率集成；
- Linux 上全部 12 种颜色输出、线程安全、自动初始化选项组合；
- 发布校验和打包回归测试。

每种配置都显式开启测试。任一必需任务失败、取消或跳过，`CI result` 都会失败。
日常平台矩阵使用 Debug；发布时三个平台同时验证 Debug 和 Release。
CI 不再调用旧的串行 `build-all.sh` 矩阵。

## 发布版本

1. 向 `dev` 提交 PR，更新 `include/zeroerr/internal/config.h` 中的
   `ZEROERR_VERSION_MAJOR/MINOR/PATCH`，重新生成 `zeroerr.hpp`，通过 CI 后合并。
2. 创建 `dev` → `master` 提升 PR，等待 `PR target` 和 `CI result` 通过，
   使用 merge commit 合并。
3. 为提升后的提交打标签（将 `0.4.2` 替换为实际版本）：

   ```sh
   git fetch origin master
   git tag -a v0.4.2 origin/master -m 'zeroerr v0.4.2'
   git push origin v0.4.2
   ```

4. Release 工作流校验正式版 `v主版本.次版本.补丁版本` 标签、两处版本声明，
   并确认提交已在 `master` 历史中。随后固定提交 SHA，复用完整 CI，
   在三个系统上同时运行 Debug 和 Release。
5. 全部测试成功后，再次校验标签未移动，创建 GitHub Release，上传
   `zeroerr.hpp`、源码 `.tar.gz`/`.zip` 和 `SHA256SUMS`。
   只有发布任务具有写权限，已有 Release 不会被覆盖。

临时错误可重跑失败工作流，也可手动触发 Release 并填写已有标签；工作流分支
选择 `master`。手动触发同样要求已有、版本匹配且属于 `master` 的标签，
不会创建或移动标签。发布失败会保留标签，不会发布新版本。
如果 GitHub 上传中途失败，重试前请检查可能留下的草稿；工作流不会覆盖已有 Release。
此流程不包含预发布版本或自动递增版本号。
