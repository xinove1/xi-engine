#ifndef XI_INPUT_ACTIONS_H
# define XI_INPUT_ACTIONS_H

# include "types.h"

// Comment out or redefine if needed
typedef enum {LEFT, RIGHT, UP, DOWN, ACTION_1, ACTION_2, ACTION_3, OPEN_MENU, INPUT_ACTION_COUNT} InputActions_e;

void RegisterActionName(i32 action_id, byte *action_name);
void RegisterInputKeyAction(i32 action_id, i32 keycode);
void RegisterGamePadButtonAction(i32 action_id, i32 gamepad_button);
void RegisterGamePadAxisAction(i32 action_id, i32 gamepad_axis, f32 trigger_deadzone);
void SetGamePadId(i32 gamepad); // Which GamePad to use
bool IsActionPressed(i32 action_id);
bool IsActionReleased(i32 action_id);
bool IsActionDown(i32 action_id);
bool IsMouseMoving();
bool WasAnyActionDown();
void PoolActions();
void PrintActions();

# ifndef MAX_ACTION_KEYCODES 
#  define MAX_ACTION_KEYCODES 10
# endif
# ifndef MAX_ACTION_AXIS 
#  define MAX_ACTION_AXIS 2
# endif
# ifndef MAX_ACTION_GAMEPADBUTTONS
#  define MAX_ACTION_GAMEPADBUTTONS 4
# endif
# ifndef MAX_ACTION
#  define MAX_ACTIONS 10
# endif

#endif

#ifdef XI_INPUT_ACTIONS_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)

typedef struct {
	i32 id;
	f32 trigger_point;
} ActionAxis;

typedef struct {
	char	   *name;
	b32	   down;
	b32	   down_last_frame;
	b32	   pressed;
	b32	   released;
	i32	   keycodes[MAX_ACTION_KEYCODES];
	i32	   gamepad_button[MAX_ACTION_GAMEPADBUTTONS];
	ActionAxis gamepad_axis[MAX_ACTION_AXIS];
} Action;

Action  Actions[MAX_ACTIONS] = {0};
i32	GamePadId = -1;
Vector2 MousePosition = {0, 0};
Vector2 MouseLastPosition = {0, 0};
b32	MouseMoving = false;
b32	WasActionDown = false;

// Which GamePad to use
void SetGamePadId(i32 gamepad)
{
	if (gamepad < 0) {
		TraceLog(LOG_WARNING, "Trying te set gamepad to a negative number\n");
	}
	GamePadId = gamepad;
}

bool _CheckDown(Action *action) 
{
	for (i32 i = 0; i < MAX_ACTION_KEYCODES; i++) {
		if (action->keycodes[i] == -1) break ;

		if (IsKeyDown(action->keycodes[i])) {
			return true;
		}
	}

	// Don't check Gamepad stuff if it's unavailable
	if (IsGamepadAvailable(GamePadId) == false) {
		return false;
	}

	for (i32 i = 0; i < MAX_ACTION_GAMEPADBUTTONS; i++) {
		if (action->gamepad_button[i] == -1) break ;

		if (IsGamepadButtonDown(GamePadId, action->gamepad_button[i])) {
			return true;
		}
	}

	for (i32 i = 0; i < MAX_ACTION_AXIS; i++) {
		if (action->gamepad_axis[i].id == -1) break ;

		f32	f = GetGamepadAxisMovement(GamePadId, action->gamepad_axis[i].id);
		f32	trigger = action->gamepad_axis[i].trigger_point;
		if (trigger > 0) {
			if (f >= action->gamepad_axis[i].trigger_point) {
				return true;
			}
		} else if (trigger < 0) {
			if (f <= action->gamepad_axis[i].trigger_point) {
				return true;
			}
		} else {
			TraceLog(LOG_WARNING, "Action |%s| gamepad_axis_id %d has a trigger poi32 of 0 \n", action->name, action->gamepad_axis[i].id);
		}
	}
	
	return false;
}

void PoolActions() 
{
	WasActionDown = false;
	for (i32 i = 0; i < MAX_ACTIONS; i++) {
		Action	*action = &Actions[i];
		if (action->name == NULL) continue ;
		
		action->down_last_frame = action->down;
		action->down = _CheckDown(action);
		action->pressed = false;
		action->released = false;
		if (action->down && action->down_last_frame == false) {
			action->pressed = true;
		} else if (action->down_last_frame && action->down == false) {
			action->released = true;
		}

		if (action->down_last_frame) {
			WasActionDown = true;
		}
	}
	MouseMoving = false;
	MouseLastPosition = MousePosition;
	MousePosition = GetMousePosition();
	if (!(MousePosition.x == MouseLastPosition.x && MousePosition.y == MouseLastPosition.y)) {
		MouseMoving = true;
	}
}

Action	*_GetAction(i32 id) 
{
	// TODO  Better error message to account to error on registering and getting input
	if (id < 0) {
		TraceLog(LOG_WARNING, "Tryng to register a Key action with negative value, aborting.\n");
		return (NULL);
	} else if (id >= MAX_ACTIONS) {
		TraceLog(LOG_WARNING, "Tryng to register a Key action with value greater/equal than MAX_ACTIONS, aborting.\n");
		return (NULL);
	}
	return (&Actions[id]);
}

void RegisterActionName(i32 action_id, byte *action_name)
{
	if (action_name == NULL) {
		TraceLog(LOG_WARNING, "Tryng to register a action with null name, aborting.\n");
		return ;
	}
	
	Action *action = _GetAction(action_id);
	if (action == NULL) {
		return ;
	}

	if (action->name == NULL) {
		TraceLog(LOG_DEBUG, "Action \"%s\" does not exists, registering new action.\n", action_name);
		action->name = action_name;
	} else {
		TraceLog(LOG_DEBUG, "Overwriting action name \"%s\" with \"%s\".\n", action->name, action_name);
		action->name = action_name;
	}

	action->down_last_frame = false;
	action->down = false;

	for (i32 i = 0; i < MAX_ACTION_KEYCODES; i++) {
		action->keycodes[i] = -1;
	}

	for (i32 i = 0; i < MAX_ACTION_GAMEPADBUTTONS; i++) {
		action->gamepad_button[i] = -1;
	}

	for (i32 i = 0; i < MAX_ACTION_AXIS; i++) {
		action->gamepad_axis[i].id = -1;
	}
}

// Register a keycode to a action, only works on action already named (so the keycodes can be set to -1)
void RegisterInputKeyAction(i32 action_id, i32 action_keycode)
{
	Action *action = _GetAction(action_id);
	if (action == NULL) {
		return ;
	}

	for (i32 i = 0; i < MAX_ACTION_KEYCODES; i++) {
		if (action->keycodes[i] == -1) {
			action->keycodes[i] = action_keycode;
			return ;
		}
	}
	
	char *action_name = action->name;
	if (action_name == NULL) {
		action_name = (char *)"Unamned action";
	}

	TraceLog(LOG_WARNING, "Action \"%s\" keycodes are full. See MAX_ACTION_KEYCODES macro.\n", action_name);
	return ;
}

void RegisterGamePadButtonAction(i32 action_id, i32 gamepad_button) 
{
	Action *action = _GetAction(action_id);
	if (action == NULL) {
		return ;
	}

	for (i32 i = 0; i < MAX_ACTION_GAMEPADBUTTONS; i++) {
		if (action->gamepad_button[i] == -1) {
			action->gamepad_button[i] = gamepad_button;
			return ;
		}
	}
	
	char *action_name = action->name;
	if (action_name == NULL) {
		action_name = (char *)"Unamned action";
	}

	TraceLog(LOG_WARNING, "Action \"%s\" GamepadButtons are full. See MAX_ACTION_GAMEPADBUTTONS macro.\n", action_name);
	return ;
}

void RegisterGamePadAxisAction(i32 action_id, i32 gamepad_axis, f32 trigger_point)
{
	Action *action = _GetAction(action_id);
	if (action == NULL) {
		return ;
	}

	for (i32 i = 0; i < MAX_ACTION_AXIS; i++) {
		if (action->gamepad_axis[i].id == -1) {
			action->gamepad_axis[i].id = gamepad_axis;
			action->gamepad_axis[i].trigger_point = trigger_point;
			return ;
		}
	}
	
	char *action_name = action->name;
	if (action_name == NULL) {
		action_name = (char *)"Unamned action";
	}

	TraceLog(LOG_WARNING, "Action \"%s\" GamepadAxis are full. See MAX_ACTION_AXIS macro.\n", action_name);
	return ;
}


bool IsActionPressed(i32 action_id)
{
	Action *action = _GetAction(action_id);
	if (action == NULL) {
		TraceLog(LOG_WARNING, "IsActionPressed: action_id %d, not found", action_id);
		return false;
	}
	return (action->pressed);
}

bool IsActionReleased(i32 action_id)
{
	Action *action = _GetAction(action_id);
	if (action == NULL) {
		TraceLog(LOG_WARNING, "IsActionReleased: action_id %d, not found", action_id);
		return false;
	}
	return (action->released);
}

bool IsActionDown(i32 action_id)
{
	Action *action = _GetAction(action_id);
	if (action == NULL) {
		TraceLog(LOG_WARNING, "IsActionDown: action_id %d, not found", action_id);
		return false;
	}
	return (action->down);
}

bool IsMouseMoving() 
{
	return (MouseMoving);
}

bool WasAnyActionDown()
{
	return (WasActionDown);
}

void PrintActions() {
	TraceLog(LOG_INFO, "Actions list\n");
	TraceLog(LOG_INFO, "~~~~~~~~~~~~\n");
	for (i32 i = 0; i < MAX_ACTIONS; i++) {
		if (Actions[i].name == NULL) {
			TraceLog(LOG_INFO, "Unamned action: \n");
		} else {
			TraceLog(LOG_INFO, "%s: \n", Actions[i].name);
		}
		for (i32 k = 0; k < MAX_ACTION_KEYCODES; k++) {
			if (Actions[i].keycodes[k] == -1) break ;
			TraceLog(LOG_INFO, "  %d\n", Actions[i].keycodes[k]);
		}
	}
}

// NOLINTEND(misc-definitions-in-headers)
#endif 
