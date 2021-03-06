namespace Settings
{
	
	const float IconSize = 22;
	
	const float ToolbarIconSize = 33;
	const float UIFadeAlpha = 0.35;
	const float UIFadeSpeed = 4;
	
	const float SelectRectLineWidth			= 1.5;
	const uint SelectRectFillColour			= 0x1144ff44;
	const uint SelectRectLineColour			= 0x55aaffaa;
	
	const float DefaultLineWidth			= 1;
	const uint DefaultLineColour			= 0x55ffffff;
	const uint DefaultFillColour			= 0x10ffffff;
	
	const float HoveredLineWidth			= 1.5;
	const uint HoveredLineColour			= 0xff44eeff;
	const uint HoveredFillColour			= 0x5544eeff;
	
	const float SelectedLineWidth			= 1;
	const uint SelectedLineColour			= 0xcc44eeff;
	const uint SelectedFillColour			= 0x3344eeff;
	
	const float PendingAddLineWidth			= 1;
	const uint PendingAddFillColour			= 0x3344ff44;
	const uint PendingAddLineColour			= 0xaa44ff44;
	
	const float PendingRemoveLineWidth		= 1;
	const uint PendingRemoveFillColour		= 0x33ff4444;
	const uint PendingRemoveLineColour		= 0xaaff4444;
	
	const float BoundingBoxLineWidth		= 1.5;
	const uint BoundingBoxColour			= 0x55ffffff;
	
	const float	RotationHandleOffset		= 18;
	
	const float	RotateHandleSize			= 5;
	const uint RotateHandleColour			= 0xaaffffff;
	
	const float	ScaleHandleSize				= 4;
	const uint RotateHandleHoveredColour	= 0xaaea9c3f;
	
	const float	SelectDiamondSize			= 3;
	
	const array<float> ScaleHandleOffsets = {
		// Right
		 1,  0,
		 1,  0,
		// Bottom Right
		 1,  1,
		 1,  1,
		// Bottom
		 0,  1,
		 0,  1,
		// Bottom Left
		-1,  1,
		-1,  1,
		// Left
		-1,  0,
		-1,  0,
		// Top Left
		-1, -1,
		-1, -1,
		// Top
		 0, -1,
		 0, -1,
		// Top Right
		 1, -1,
		 1, -1,
	};
	
	const array<int> RepeatKeys = {
		GVB::LeftArrow,
		GVB::RightArrow,
		GVB::UpArrow,
		GVB::DownArrow,
	};
	
	/// How long (frames) to pause after the initial key press before the key starts repeating
	const int KeyPressDelay = 25;
	/// While a key is pressed, this specifies the speed (in frames) at which it will trigger
	const int KeyRepeatPeriod = 2;
	
}

class Setting
{
	[text] string key;
}

class BoolSetting : Setting
{
	[text] bool value;
}

class IntSetting : Setting
{
	[text] int value;
}

class FloatSetting : Setting
{
	[text] float value;
}

class StringSetting : Setting
{
	[text] string value;
}