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
