<!-- 选择语言 / Choose a language -->

**选择语言 / Choose a language:** [中文](#中文) · [English](#english)

---

# 中文

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

---

# English

# Student

## Overview
This repository is a UE5 C++ prototype project focused on first-person gameplay, Enhanced Input, and reusable interaction systems. It is designed to support a player controller, interactable objects, and a camera-based interaction component that can be reused in both Blueprint and C++ workflows.

## Features
- AStudentPlayerCharacter / AStudentCharacter: configurable first-person character with movement, camera, sprint, and crouch behavior.
- Enhanced Input integration: movement and camera actions are bound through mapping contexts and input actions.
- Interaction system:
  - UStudentInteractableInterface with a BlueprintNativeEvent named Interact
  - UStudentInteractionComponent performing a camera-forward line trace to detect interactable actors
- Blueprints can configure InteractAction, InteractionDistance, debug visualization, and other properties.

## Requirements
- Unreal Engine 5
- C++ development tools for your platform
  - Windows: Visual Studio 2022 with C++ workload
  - macOS: Xcode if developing on macOS
- Enhanced Input module support in the project

## Quick Start
1. Clone the repository:
   git clone https://github.com/bookid284-art/Student.git
2. Open the .uproject file in Unreal Editor.
3. Generate project files if needed.
4. Compile the project in Visual Studio or through the Unreal Editor.
5. Open a level and run the project to test the player character and interaction behavior.

## Example Usage
- Add the Student player character to your level or set it as the default pawn in the Game Mode.
- Configure an Input Mapping Context with MoveAction, LookAction, RunAction, and CrouchAction.
- Use InteractionComponent to adjust InteractionDistance and debug visualizations.
- Implement the Interact function on any actor that should be interactive.

## Project Structure
- Source/Student/: C++ source files for the gameplay systems and module definition
- Content/: engine assets, blueprints, and input assets

## Notes
- This project includes the core gameplay systems but does not currently include automated tests.
- Different UE5 versions may require regenerating project files and validating build dependencies.

## Contributing
Issues and pull requests are welcome. Please include a short description of the change and testing notes in your PR.

## License
No license is currently specified in the repository. Add a LICENSE file if you want to publish the project under an explicit open-source license.
