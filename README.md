# Student

## 简要说明
- 基于 Unreal Engine 5 的 C++ 演示/原型项目，目标是提供第一人称玩家控制、增强输入（Enhanced Input）支持与交互系统（Interactable Interface、Interaction Component），方便在蓝图或 C++ 层面复用。

## 主要功能
- AStudentPlayerCharacter / AStudentCharacter：可配置的第一人称角色，支持移动、视角、跑/蹲状态切换。
- Enhanced Input 集成：通过 Input Mapping Context 与 Input Actions 进行绑定。
- 交互系统：
  - UStudentInteractableInterface（BlueprintNativeEvent Interact）
  - UStudentInteractionComponent：相机前方射线检测可交互对象并触发交互事件。
- 可在蓝图中设置 InteractAction、调整 InteractionDistance 和调试展示。

## 需求
- Unreal Engine 5（建议使用官方稳定的 UE5 版本）。
- 支持 C++ 的开发工具链：
  - Windows: Visual Studio 2022（含 C++ 工作负载）或等效工具
  - macOS: Xcode（如果在 macOS 下开发）
- 项目依赖 EnhancedInput 模块（已在 Student.Build.cs 中声明）。

## 快速开始（编译与运行）
1. 克隆仓库：
   git clone https://github.com/bookid284-art/Student.git
2. 在项目根目录右键 -> Generate Visual Studio project files（或用 Unreal Editor 打开 .uproject 并选择生成项目文件）。
3. 使用 Visual Studio 打开生成的解决方案，编译项目（或在 Editor 中点击 Compile）。
4. 在 Unreal Editor 中打开关卡并运行。确保你的本地玩家配置包含 DefaultMappingContext、并在蓝图/实例中为 PlayerCharacter 指定 Input Actions。

## 示例使用（在编辑器内）
- 将 StudentPlayerCharacter 放入关卡或设置为 GameMode 的默认 Pawn。
- 在角色蓝图或属性面板中：
  - 指定 DefaultMappingContext（Enhanced Input）
  - 设置 MoveAction、LookAction、RunAction、CrouchAction
  - 为 InteractionComponent 调整 InteractionDistance 并启用调试显示以观察射线检测结果
- 在想要被交互的 Actor 上实现 StudentInteractableInterface 并实现 Interact 方法（Blueprint 或 C++）。

## 项目结构（示例）
- Source/Student/ - C++ 源代码（StudentCharacter, StudentPlayerCharacter, StudentInteractionComponent, StudentInteractableInterface, Student.Build.cs）
- Content/ - 引擎资源（关卡、蓝图、输入资产等）

## 已知问题与注意事项
- 当前合并的 PR 包含核心功能但未添加自动化测试。运行前请确保项目能在你本地的 UE5 版本上成功编译。
- 若使用不同的 UE5 次版本（例如 5.1 vs 5.2），可能需要重新生成项目文件并调整模块依赖。

## 贡献
- 欢迎提交 issue 或 PR。请在 PR 描述中说明更改动机并附带简短测试说明（本地构建/编辑器运行验证）。

## 许可证
- 若需公开许可，请在仓库根目录添加 LICENSE（常见：MIT / Apache-2.0 / GPL）。
