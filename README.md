# MSDV_SpaceInvaders

This project recreates the classic Space Invaders game in Unreal Engine 5.4 using C++ and make use of its own GameModeBase, Pawn, HUD, PlayerController and GameInstance. Its features the use of Niagara effects and EnhancedInput as well.

https://github.com/user-attachments/assets/a8e519d6-73cb-4307-be67-a12f9880a4b9

`SIGameModeBase` will generate a squad at the beginning of the game or when the player destroy one. This GameModeBase counts with multiple delegates to warn all subscribers when an invader have been destroyed, when a squad has reached the right/left side of the screen, etc.
`SIPawn` will bind all inputs to a function in charge of performing the action. These are basic actions such as firing or moving. It also handles the score and the health points.
<img width="1919" height="1004" alt="image" src="https://github.com/user-attachments/assets/d74f5a85-786d-4bb6-a014-612777be8491" />


`BP_HUD` creates the widget blueprint that will display the player score and its health points.
<img width="2258" height="1328" alt="image" src="https://github.com/user-attachments/assets/98a705cf-a5b7-434c-971f-07e4683ed710" />

`SIGameInstance` will store the highest score got across all levels (main menu and game).

`Invader` has been defined for the invader logic and the blueprint that implements this class (`BP_Invader`) has a custom movement component attached to it (`InvaderMovementComponent`). This compoment make use of an enum to identify how it should be moved each frame.

```c++
enum class InvaderMovementType : uint8
{
	STOP = 0 UMETA(DisplayName = "Stopped"),
	RIGHT = 1 UMETA(DisplayName = "Right"),
	LEFT = 2 UMETA(DisplayName = "Left"),
	DOWN = 3 UMETA(DisplayName = "Down"),
	FREEJUMP = 4 UMETA(DisplayName = "Free Jump")
};
```

Each `Invader` is part of a `InvaderSquad` that will update their movement state. If the state is `FREEJUMP` this means that the invader will leave the squad to move towards the player while firing to crash into him.


<img width="1919" height="1004" alt="image" src="https://github.com/user-attachments/assets/25f58127-41cb-4c5f-9562-3f2e7e284dd0" />


<img width="1181" height="409" alt="image" src="https://github.com/user-attachments/assets/a920777a-13af-4ff3-826e-09daedc91ec9" />





