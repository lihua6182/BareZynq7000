# 代码风格与约定

本文档记录本仓库的编码约定。修改第一方代码前先读一遍；能交给工具处理的部分不要手工争论。

## 1. 工具

| 工具 | 约定 |
| --- | --- |
| clang-format | 23.x（建议固定大版本）；配置见 `.clang-format` |
| GCC | `arm-none-eabi-gcc`，告警集见 `scripts/cmake/gcc-arm-toolchain.cmake` |
| 编辑器 | `.editorconfig`：UTF-8、LF、4 空格、文件末尾换行、去尾随空格 |

```sh
clang-format --style=file -i <源文件>  # 直接对第一方 C 源码执行
```

第三方目录由 `.clang-format-ignore` 排除（`source/middleware/cherryusb` 和
`source/middleware/lzma/sdk`，保持上游格式，不参与格式化）。如果系统里没有
`clang-format`，设置 `CLANG_FORMAT` 环境变量指向可执行文件。

## 2. 排版

- 大括号：Allman。函数、`if`/`for`/`while`/`switch` 的左括号独占一行。
- 缩进：4 空格，禁止 Tab。
- 文件以内容开头，不留首行空行；文件末尾保留一个换行。
- 换行符统一 LF，不要把 CRLF 带进提交。
- 空行只用于分隔逻辑段，不要保留“删注释后”的孤立空行。

## 3. 命名

- 函数、变量、类型：`snake_case`；类型用 `struct xxx` / `enum xxx`，不加 `_t` 后缀。
- 公开符号带模块前缀：`zynq_*`、`board_*`、`mio_*`、`pl_*`、`pcap_*`、`uart_*` 等。
- 文件内 `static` 辅助函数也必须带模块前缀（如 `i2c_wait_complete`），不要用 `gate_bit` 这类裸名。
- 宏：`UPPER_SNAKE_CASE`。
- 寄存器字段：沿用 TRM/CMSIS 风格，多位字段用 `_Msk` + `_Pos`，单比特字段直接用位值宏
  （如 `UART_SR_TXFULL`）。
- 枚举类型名小写。硬件标识枚举值也用小写（`fclk0`、`usb0_irq`）；状态/错误枚举值用
  `UPPER_SNAKE_CASE`（`PL_OK`、`LZMA_STREAM_OK`）。
- 结构体成员小写加下划线，按寄存器/硬件命名，不随意缩写。

## 4. 字面量

- 32 位无符号常量统一 `U` 后缀：`0x1U`、`4096U`、`0xFFFFFFFFU`。不要混用 `u`、`UL`、`ul`。
- 只有确实需要 64 位时才用 `ULL`。
- 十六进制前缀统一小写 `0x`，不要 `0X...`；数字本身保持大写（如 `0xF8000000U`）。
- 宏参数必须加括号；多语句宏用 `do { } while (0)`。

## 5. 类型与错误处理

- 固定宽度类型用 `<stdint.h>` 的 `uint32_t` 等；同一模块内长度参数保持一致。
- 对外布尔查询返回 `bool`（`pcap_is_ready`、`uart_tx_ready`）；需要报告非法参数时保留 `int`，
  用 `-1` 表示错误（`pl_reset_is_asserted`）。
- 错误返回：驱动层 `0` 成功 / `-1` 失败；板级状态机用 `enum pl_status`。
- 指针判空用 `NULL`；无限循环统一 `for (;;)`。
- 函数内声明放在块首。C23 允许混排，但本项目保持块首声明。

## 6. 头文件

- 保护宏：`ZYNQ_<MODULE>_H` / `BOARD_<MODULE>_H`。
- include 顺序：自身头文件、空行、系统头文件、空行、项目头文件；块内按字母序。
- 只包含用得到的头文件，不重复包含。
- 公开头文件只放声明和 `static inline` 小函数，不放实现。

## 7. 注释

- 注释解释“为什么”，不复述代码。
- 不保留空的注释占位或删注释后的孤立空行。
- 生成文件（如 `board_pl_bitstream.*`）由 `tools/convert_bitstream.py`（VSCode 任务
  Convert Bitstream）生成，不要手改。

## 8. 分层

- `soc/`：寄存器定义、启动、时钟、MMU、IRQ、tick。
- `driver/`：片上外设驱动，只依赖 `soc/`。
- `board/`：板级引脚、PL 加载、控制台。
- `app/`：应用与 newlib 桩。
- `middleware/`：LZMA 包装与第三方 USB 栈。
