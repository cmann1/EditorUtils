#include '../../enums/VK.cpp';
#include '../../enums/GVB.cpp';
#include '../../editor/common.cpp';
#include '../../input/Keyboard.cpp';
#include '../../string.cpp';
#include 'FocusableElement.cpp';

namespace TextBox { const string TYPE_NAME = 'TextBox'; }

class TextBox : FocusableElement, IStepHandler, IKeyboardFocus, INavigable
{
	
	// ///////////////////////////////////////////////////////////////////
	// Basic properties
	// ///////////////////////////////////////////////////////////////////
	
	protected string _font;
	protected uint _size;
	protected float _text_scale;
	protected float _line_spacing = 6;
	protected bool _multi_line = true;
	protected bool _smart_home = true;
	protected bool _remove_lines_shortcut = true;
	protected bool _clipboard_enabled = true;
	protected bool _accept_on_blur = true;
	protected bool _revert_on_cancel = true;
	protected bool _select_all_on_focus;
	protected bool _deselect_all_on_blur;
	protected bool _drag_scroll = true;
	
	protected float padding_left;
	protected float padding_right;
	protected float padding_top;
	protected float padding_bottom;
	
	protected float _text_width;
	protected float _text_height;
	protected float scroll_max_x;
	protected float scroll_max_y;
	
	protected int _selection_start;
	protected int _selection_end;
	
	/// Triggered anytime the text changes.
	Event change;
	/// Triggered when the value is accepted either with enter or ctrl+Enter.
	/// The event type will be CANCEL when cancelled with esacpe
	Event accept;
	
	// ///////////////////////////////////////////////////////////////////
	// Text related
	// ///////////////////////////////////////////////////////////////////
	
	protected float unscaled_line_height;
	protected array<float>@ font_metrics;
	
	protected array<string> lines;
	protected array<int> line_end_indices;
	/// Stores the length from the start of the line to the end of each character.
	/// Allows performing a binary search to quickly find the position of a character within a line
	protected array<array<float>@> line_character_widths;
	protected int _num_lines;
	protected int _text_length;
	
	// ///////////////////////////////////////////////////////////////////
	// Navigation and interaction
	// ///////////////////////////////////////////////////////////////////
	
	protected int persist_caret_time;
	
	protected bool busy_drag_scroll;
	protected float drag_scroll_start_x;
	protected float drag_scroll_start_y;
	protected float drag_mouse_x_start;
	protected float drag_mouse_y_start;
	/// Track when the textbox was scrolled, so that the selection can be update
	/// when dragging a selection with the mouse
	protected bool scrolled;
	
	protected bool drag_selection;
	protected int double_click_start_index = -1;
	protected int double_click_end_index   = -1;
	
	/// The stored relative index of the caret within the selected line.
	/// When navigating up or down a line, the caret will try to match this position.
	/// Only updated when the position within the line is explicitly changed, e.g. moving left/right, or selecting with the mouse.
	protected int line_relative_caret_index = -1;
	
	protected bool suppress_change_event;
	protected string previous_text;
	
	TextBox(UI@ ui, const string text='', const string font='', const uint size=0, const float text_scale=NAN)
	{
		super(ui);
		
		_width  = _set_width  = 140;
		_height = _set_height = 34;
		
		_font = font;
		_size = size;
		_text_scale = is_nan(text_scale) ? ui.style.default_text_scale : text_scale;
		
		_multi_line = text.findFirstOf('\n\r') != -1;
		_select_all_on_focus = !_multi_line;
		_deselect_all_on_blur = !_multi_line;
		
		ui.get_font_metrics(_font, _size, @font_metrics, unscaled_line_height);
		
		// Initialise with a single empty line
		_text_height = unscaled_line_height * _text_scale;
		lines.insertLast('');
		line_end_indices.insertLast(0);
		line_character_widths.insertLast(array<float> = {});
		
		_num_lines = 1;
		_text_length = 0;
		
		padding = ui.style.spacing;
		_navigate_on = NavigateOn(_navigate_on | (_multi_line ? CtrlReturn : Return) | Escape);
		
		suppress_change_event = true;
		this.text = text;
	}
	
	string element_type { get const { return TextBox::TYPE_NAME; } }
	
	// ///////////////////////////////////////////////////////////////////
	// Basic properties
	// ///////////////////////////////////////////////////////////////////
	
	/// Get or set this TextBox's text
	string text
	{
		get const { return join(lines, '\n'); }
		set
		{
			do_replace(0, _text_length, value);
		}
	}
	
	/// The length of the text
	int text_length
	{
		get const { return _text_length; }
	}
	
	/// The number of lines of text
	int num_lines
	{
		get const { return _num_lines; }
	}
	
	string font
	{
		get const { return _font; }
		set
		{
			if(_font == value)
				return;
			
			set_font(value, _size);
		}
	}
	
	uint size
	{
		get const { return _size; }
		set
		{
			if(_size == value)
				return;
			
			set_font(_font, value);
		}
	}
	
	void set_font(const string font, const uint size)
	{
		if(_font == font && _size == size)
			return;
		
		_font = font;
		_size = size;
		
		ui.get_font_metrics(_font, _size, @font_metrics, unscaled_line_height);
		recalculate_text_width();
		recalculate_text_height();
	}
	
	/// The scale of the text
	float text_scale
	{
		get const { return _text_scale; }
		set
		{
			if(_text_scale == value)
				return;
			
			_text_scale = value;
			recalculate_text_width();
			recalculate_text_height();
		}
	}
	
	/// The amount of spacing between each line
	float line_spacing
	{
		get const { return _line_spacing; }
		set
		{
			if(_line_spacing == value)
				return;
			
			_line_spacing = value;
			recalculate_text_height();
		}
	}
	
	/// Can this TextBox contain more than one line?
	bool multi_line
	{
		get const { return _multi_line; }
		set
		{
			if(_multi_line == value)
				return;
			
			_multi_line = value;
			
			if(_multi_line)
			{
				if((_navigate_on | NavigateOn::Return) != 0)
				{
					_navigate_on = NavigateOn((_navigate_on & ~NavigateOn::Return) | NavigateOn::CtrlReturn);
				}
			}
			else if((_navigate_on | NavigateOn::CtrlReturn) != 0)
			{
				_navigate_on = NavigateOn((_navigate_on & ~NavigateOn::CtrlReturn) | NavigateOn::Return);
			}
			
			if(!_multi_line && _num_lines > 1)
			{
				lines.resize(1);
				line_end_indices.resize(1);
				line_character_widths.resize(1);
				
				_num_lines = 1;
				_text_length = line_end_indices[0];
				
				_text_width = _text_length > 0 ? line_character_widths[0][_text_length - 1] : 0;
				recalculate_text_height();
				
				validate_selection();
			}
		}
	}
	
	/// If true pressing the home key will move the caret to the first non whitespace character in the line
	bool smart_home
	{
		get const { return _smart_home; }
		set { _smart_home = value; }
	}
	
	/// Allow removing lines with Ctrl+Shift+D
	bool remove_lines_shortcut
	{
		get const { return _remove_lines_shortcut; }
		set { _remove_lines_shortcut = value; }
	}
	
	/// Allow copy and paste with ctrl+C and ctrl+V. (Doesn't use the real clipboard for now)
	bool clipboard_enabled
	{
		get const { return _clipboard_enabled; }
		set { _clipboard_enabled = value; }
	}
	
	/// When this TextBox loses focus, accept the changes if true, otherwise cancel and revert.
	bool accept_on_blur
	{
		get const { return _accept_on_blur; }
		set { _accept_on_blur = value; }
	}
	
	/// If the escape key is pressed, the TextBox's contents will be reverted
	bool revert_on_cancel
	{
		get const { return _revert_on_cancel; }
		set { _revert_on_cancel = value; }
	}
	
	/// When this TextBox gains focus, all text will automatically be selected
	bool select_all_on_focus
	{
		get const { return _select_all_on_focus; }
		set { _select_all_on_focus = value; }
	}
	
	/// When this TextBox loses focus, all text will be deselected
	bool deselect_all_on_blur
	{
		get const { return _deselect_all_on_blur; }
		set { _deselect_all_on_blur = value; }
	}
	
	/// If true dragging with right mouse will scroll the TextBox
	bool drag_scroll
	{
		get const { return _drag_scroll; }
		set { _drag_scroll = value; }
	}
	
	/// The scaling around the inside of the TextBox
	float padding
	{
		get const { return padding_left; }
		set
		{
			if(padding_left == value)
				return;
			
			padding_left	= value;
			padding_right	= value + 1;
			padding_top		= value + ui.style.selection_padding_top;
			padding_bottom	= value + ui.style.selection_padding_bottom + 1;
		}
	}
	
	/// The total width of the text
	float text_width
	{
		get const { return _text_width; }
	}
	
	/// The total height of the text
	float text_height
	{
		get const { return _text_height; }
	}
	
	/// The height of a line of text.
	float line_height
	{
		get const { return unscaled_line_height * _text_scale; }
	}
	
	float scroll_x
	{
		set override
		{
			value = clamp_scroll(value, scroll_max_x);
			
			if(_scroll_x == value)
				return;
			
			_scroll_x = value;
		}
	}
	
	float scroll_y
	{
		set override
		{
			value = clamp_scroll(value, scroll_max_y);
			
			if(_scroll_y == value)
				return;
			
			_scroll_y = value;
		}
	}
	
	// ///////////////////////////////////////////////////////////////////
	// Selection, Navigation
	// ///////////////////////////////////////////////////////////////////
	
	int selection_start
	{
		get const { return _selection_start; }
		set
		{
			value = clamp(value, 0, _text_length);
			
			if(_selection_start == value)
				return;
			
			_selection_start = value;
		}
	}
	
	int selection_end
	{
		get const { return _selection_end; }
		set
		{
			value = clamp(value, 0, _text_length);
			
			if(_selection_end == value)
				return;
			
			_selection_end = value;
		}
	}
	
	int caret_index
	{
		get const { return _selection_end; }
		set
		{
			value = clamp(value, 0, _text_length);
			
			if(_selection_start == value && _selection_end == value)
				return;
			
			_selection_start = value;
			_selection_end = value;
		}
	}
	
	/// Sets the selection from start to end. end can be smaller than start.
	void select(int start, int end)
	{
		start = clamp(start, 0, _text_length);
		end = clamp(end, 0, _text_length);
		
		if(_selection_start == start && _selection_end == end)
			return;
		
		_selection_start = start;
		_selection_end = end;
	}
	
	/// Selects all the text.
	void select_all()
	{
		select(0, _text_length);
	}
	
	/// Sets the selection to the selection end.
	void select_none()
	{
		caret_index = _selection_end;
	}
	
	/// Returns the selected text, or an empty string
	string get_selected_text()
	{
		return get_text(_selection_start, _selection_end);
	}
	
	/// Get the text between start_index and end_index
	string get_text(int start_index, int end_index)
	{
		if(start_index == end_index)
			return '';
		
		if(end_index < start_index)
		{
			const int start_index_t = start_index;
			start_index = end_index;
			end_index = start_index_t;
		}
		
		if(start_index < 0)
		{
			start_index = 0;
		}
		
		if(end_index > _text_length)
		{
			end_index = _text_length;
		}
		
		if(start_index == 0 && end_index == _text_length)
			return text;
		
		const int start_line = get_line_at_index(start_index);
		const int end_line   = get_line_at_index(end_index);
		
		string buffer = '';
		bool add_new_line = false;
		
		for(int i = start_line; i <= end_line; i++)
		{
			if(add_new_line)
			{
				buffer += '\n';
			}
			else
			{
				add_new_line = true;
			}
			
			const int line_start_index = i > 0
				? line_end_indices[i - 1] + 1
				: 0;
			const int line_length = line_end_indices[i] - line_start_index;
			const int relative_start_index = max(0, start_index - line_start_index);
			const int relative_length      = max(0, end_index - line_start_index) - relative_start_index;
			
			buffer += relative_length < line_length || relative_start_index != 0
				? lines[i].substr(relative_start_index, relative_length)
				: lines[i];
		}
		
		return buffer;
	}
	
	/// Scrolls the caret into view. padding_x controls approximately how many extra characters
	/// will be scrolled when the caret is not in view horizontally
	void scroll_to_caret(const int padding_x=0)
	{
		if(padding_x < 0)
			return;
		
		float x1, y1;
		get_index_xy(_selection_end, x1 ,y1);
		
		scroll_into_view(
			x1 - ui.style.caret_width * 0.5,
			y1 - ui.style.selection_padding_top,
			x1 + ui.style.caret_width * 0.5,
			y1 + unscaled_line_height * text_scale + ui.style.selection_padding_bottom,
			padding_x);
	}
	
	/// Scrolls the given rectangle into view. padding_x controls approximately how many extra characters
	/// will be scrolled when the rect is not in view horizontally
	void scroll_into_view(const float x1, const float y1, const float x2, const float y2, const int padding_x=0)
	{
		const float view_width  = _width  - padding_left - padding_right;
		const float view_height = _height - padding_top - padding_bottom;
		
		if(_scroll_x + x1 < 0)
		{
			scroll_x = -x1 + max(0, padding_x) * font_metrics[0] * text_scale;
		}
		else if(_scroll_x + x2 > view_width)
		{
			scroll_x = view_width - x2 - max(0, padding_x) * font_metrics[0] * text_scale;
		}
		
		if(_scroll_y + y1 < 0)
		{
			scroll_y = -y1;
		}
		else if(_scroll_y + y2 > view_height)
		{
			scroll_y = view_height - y2;
		}
	}
	
	/// If word is true moves to the next word boundary. extend controls wether to move the caret, or extend the selection
	void move_caret_left(const bool word, const bool extend, const bool scroll_to_caret=false)
	{
		const int index = word
			? find_word_boundary(_selection_end, -1)
			: _selection_end - 1;
		
		if(extend)
		{
			selection_end = index;
		}
		else
		{
			caret_index = index;
		}
		
		this.scroll_to_caret(scroll_to_caret ? 8 : -1);
		line_relative_caret_index = -1;
		persist_caret();
	}
	
	/// If word is true moves to the next word boundary. extend controls wether to move the caret, or extend the selection
	void move_caret_right(const bool word, const bool extend, const bool scroll_to_caret=false)
	{
		const int index = word
			? find_word_boundary(_selection_end, 1)
			: _selection_end + 1;
		
		if(extend)
		{
			selection_end = index;
		}
		else
		{
			caret_index = index;
		}
		
		this.scroll_to_caret(scroll_to_caret ? 8 : -1);
		line_relative_caret_index = -1;
		persist_caret();
	}
	
	/// Moves the caret up a line. extend controls wether to move the caret, or extend the selection
	void move_caret_up(const bool extend, const bool scroll_to_caret=false)
	{
		const int selection_end_line = update_relative_line_index();
		
		if(selection_end_line == 0)
			return;
			
		update_relative_line_index();
		
		const int line_start = get_line_start_index(selection_end_line - 1);
		const int line_end   = get_line_end_index  (selection_end_line - 1);
		const int index = clamp(line_start + line_relative_caret_index, line_start, line_end);
		
		if(extend)
		{
			selection_end = index;
		}
		else
		{
			caret_index = index;
		}
		
		if(scroll_to_caret)
		{
			this.scroll_to_caret();
		}
		
		persist_caret();
	}
	
	/// Moves the caret down a line. extend controls wether to move the caret, or extend the selection
	void move_caret_down(const bool extend, const bool scroll_to_caret=false)
	{
		const int selection_end_line = update_relative_line_index();
		
		if(selection_end_line == _num_lines - 1)
			return;
		
		const int line_start = get_line_start_index(selection_end_line + 1);
		const int line_end   = get_line_end_index  (selection_end_line + 1);
		const int index = clamp(line_start + line_relative_caret_index, line_start, line_end);
		
		if(extend)
		{
			selection_end = index;
		}
		else
		{
			caret_index = index;
		}
		
		if(scroll_to_caret)
		{
			this.scroll_to_caret();
		}
		
		persist_caret();
	}
	
	/// Moves the caret to the start of the line, or the beginning of the text if start is true.
	/// extend controls wether to move the caret, or extend the selection
	void move_caret_home(const bool start, const bool extend, const bool scroll_to_caret=false)
	{
		const int selection_end_line = get_line_at_index(_selection_end);
		const int line_start = start
			? 0
			: get_line_start_index(selection_end_line);
		
		int home_index = line_start;
		
		if(_smart_home && !start)
		{
			const int line_end = get_line_end_index(selection_end_line);
			const string line = lines[selection_end_line];
			const int length = int(line.length());
			
			for(int i = 0; i < length; i++)
			{
				if(!string::is_whitespace(line[i]))
					break;
				
				home_index++;
			}
			
			if(_selection_end == home_index)
			{
				home_index = line_start;
			}
		}
		
		if(extend)
		{
			selection_end = home_index;
		}
		else
		{
			caret_index = home_index;
		}
		
		if(scroll_to_caret)
		{
			this.scroll_to_caret();
		}
		
		line_relative_caret_index = -1;
		persist_caret();
	}
	
	/// Moves the caret to the end of the line, or the end of the text if end is true.
	/// extend controls wether to move the caret, or extend the selection
	void move_caret_end(const bool end, const bool extend, const bool scroll_to_caret=false)
	{
		const int line_end = end
			? _text_length
			: get_line_end_index(get_line_at_index(_selection_end));
		
		if(extend)
		{
			selection_end = line_end;
		}
		else
		{
			caret_index = line_end;
		}
		
		if(scroll_to_caret)
		{
			this.scroll_to_caret();
		}
		
		line_relative_caret_index = -1;
		persist_caret();
	}
	
	// ///////////////////////////////////////////////////////////////////
	// Text manipulation
	// ///////////////////////////////////////////////////////////////////
	
	/// Clears all text
	void clear()
	{
		const bool changed = _text_length > 0;
		
		lines.resize(1);
		line_end_indices.resize(1);
		line_character_widths.resize(1);
		
		lines[0] = '';
		line_end_indices[0] = 0;
		line_character_widths[0].resize(0);
		
		_num_lines = 1;
		_text_length = 0;
		
		_text_width = 0;
		recalculate_text_height();
		validate_selection();
		
		dispatch_change_event(changed);
	}
	
	/// Removes all selected text. Returns the number of removed characters.
	/// If scroll_to_caret >= 0 the caret will be kept approximately that many characters in view.
	int remove(const bool move_caret_to_end=false, const int scroll_to_caret=-1)
	{
		return remove(_selection_start, _selection_end, move_caret_to_end, scroll_to_caret);
	}
	
	/// Removes the text between start_index and end_index. Returns the number of removed characters.
	/// If scroll_to_caret >= 0 the caret will be kept approximately that many characters in view.
	int remove(const int start_index, const int end_index, const bool move_caret_to_end=false, const int scroll_to_caret=-1)
	{
		const int count = do_replace(start_index, end_index, '') & 0xffffffff;
		
		if(count != 0)
		{
			adjust_selection(min(start_index, end_index), -count, move_caret_to_end);
			this.scroll_to_caret(scroll_to_caret);
		}
		
		return count;
	}
	
	/// Removes all lines between start_line_index and end_line_index (inclusive).
	void remove_lines(int start_line_index, int end_line_index, const int scroll_to_caret=-1)
	{
		start_line_index	= clamp(start_line_index, 0, num_lines - 1);
		end_line_index		= clamp(end_line_index, 0, num_lines - 1);
		
		if(end_line_index < start_line_index)
		{
			const int start_line_index_t = start_line_index;
			start_line_index = end_line_index;
			end_line_index = start_line_index_t;
		}
		
		const int remove_line_count = end_line_index - start_line_index + 1;
		
		if(_num_lines == remove_line_count)
		{
			clear();
			return;
		}
		
		const int new_caret_index = get_line_start_index(start_line_index);
		int remove_length = 0;
		
		for(int line_index = start_line_index; line_index <= end_line_index; line_index++)
		{
			remove_length += lines[line_index].length() + 1;
		}
		
		// Remove a character for each newline
		_text_length -= _num_lines - 1;
		
		_num_lines -= remove_line_count;
		_text_length -= remove_length;
		// Add a character for each newline
		_text_length += _num_lines - 1;
		
		// Remove lines from arrays
		if(remove_line_count > 0)
		{
			// removeRange seems to be broken so manually remove
			for(int j = start_line_index; j < _num_lines; j++)
			{
				lines[j]					= lines[j + remove_line_count];
				@line_character_widths[j]	= @line_character_widths[j + remove_line_count];
				line_end_indices[j]			= line_end_indices[j + remove_line_count];
			}
			
			lines.resize(_num_lines);
			line_character_widths.resize(_num_lines);
			line_end_indices.resize(_num_lines);
			
			// lines.removeRange(start_line_index, remove_line_count);
			// line_end_indices.removeRange(start_line_index, remove_line_count);
			// line_character_widths.removeRange(start_line_index, remove_line_count);
		}
		
		// Update line end indices
		for(int i = start_line_index; i < _num_lines; i++)
		{
			line_end_indices[i] -= remove_length;
		}
		
		recalculate_text_width(false);
		recalculate_text_height();
		caret_index = new_caret_index;
		this.scroll_to_caret(scroll_to_caret);
		
		dispatch_change_event();
//		debug_lines(true);
	}
	
	/// Replaces the current selection with the given ascii character.
	/// If scroll_to_caret >= 0 the caret will be kept approximately that many characters in view.
	/// Returns the number of chracters removed as the low 32 bits and the number inserted as the high 32 bits.
	int64 replace(const int chr, const bool move_caret_to_end=false, const int scroll_to_caret=-1)
	{
		return replace(string::chr(chr), _selection_start, _selection_end, move_caret_to_end, scroll_to_caret);
	}
	
	/// Replaces the current selection with the given string.
	/// If scroll_to_caret >= 0 the caret will be kept approximately that many characters in view.
	/// Returns the number of chracters removed as the low 32 bits and the number inserted as the high 32 bits.
	int64 replace(const string str, const bool move_caret_to_end=false, const int scroll_to_caret=-1)
	{
		return replace(str, _selection_start, _selection_end, move_caret_to_end, scroll_to_caret);
	}
	
	/// Replaces the specified range with str.
	/// If scroll_to_caret >= 0 the caret will be kept approximately that many characters in view.
	/// Returns the number of chracters removed as the low 32 bits and the number inserted as the high 32 bits.
	int64 replace(const string str, const int start_index, const int end_index, const bool move_caret_to_end=false, const int scroll_to_caret=-1)
	{
		const int64 count = do_replace(start_index, end_index, str);
		
		if(count != 0)
		{
			adjust_selection(min(start_index, end_index), (count >> 32) - (count & 0xffffffff), move_caret_to_end);
			this.scroll_to_caret(scroll_to_caret);
		}
		
		return count;
	}
	
	/// Insert str at the given index. Returns the number of inserted characters.
	int insert(const string str, int insert_index, const bool move_caret_to_end=false, const int scroll_to_caret=-1)
	{
		const int count = (do_replace(insert_index, insert_index, str) >> 32) & 0xffffffff;
		
		if(count != 0)
		{
			adjust_selection(insert_index, count, move_caret_to_end);
			this.scroll_to_caret(scroll_to_caret);
		}
		
		return count;
	}
	
	/// Deletes the current selection, a character in direction, or up to the next word in direction.
	void delete(const bool word, int dir=1, const int scroll_to_caret=-1)
	{
		dir = dir >= 0 ? 1 : -1;
		
		int start_index = min(_selection_start, _selection_end);
		int end_index   = max(_selection_start, _selection_end);
		
		if(start_index == end_index)
		{
			if(dir < 0)
			{
				start_index = max(0, word ? find_word_boundary(start_index, -1) : start_index - 1);
			}
			else
			{
				end_index = min(_text_length, word ? find_word_boundary(start_index, 1) : end_index + 1);
			}
			
			if(start_index == end_index)
				return;
		}
		
		do_replace(start_index, end_index, '');
		caret_index = start_index;
		this.scroll_to_caret(scroll_to_caret);
	}
	
	/// Copies the current selection to the clipboard
	void copy()
	{
		if(_selection_start == _selection_end)
			return;
		
		ui.clipboard = get_selected_text();
	}
	
	/// Cuts the current selection to the clipboard
	void cut(const int scroll_to_caret=-1)
	{
		if(_selection_start == _selection_end)
			return;
		
		ui.clipboard = get_selected_text();
		do_replace(_selection_start, _selection_end, '');
		caret_index = _selection_start;
		this.scroll_to_caret(scroll_to_caret);
	}
	
	/// Pastes from the clipboard into the current selection
	void paste(const int scroll_to_caret=-1)
	{
		const string text = ui.clipboard;
		
		if(text == '')
			return;
		
		const int64 count = do_replace(_selection_start, _selection_end, text);
		
		if(count != 0)
		{
			adjust_selection(min(_selection_start, _selection_end), (count >> 32) - (count & 0xffffffff), true);
			this.scroll_to_caret(scroll_to_caret);
		}
	}
	
	// ///////////////////////////////////////////////////////////////////
	// Misc Utility
	// ///////////////////////////////////////////////////////////////////
	
	/// Finds the start ore end of the next word from start_index in direction dir.
	int find_word_boundary(const int start_index, int dir)
	{
		dir = dir >= 0 ? 1 : -1;
		
		if(start_index <= 0 && dir == -1)
			return 0;
		if(start_index >= _text_length && dir == 1)
			return _text_length;
		
		int index = dir == 1 ? start_index : max(0, start_index - 1);
		int line_index = get_line_at_index(index);
		const int line_start_index = line_index > 0 ? line_end_indices[line_index - 1] + 1 : 0;
		index -= line_start_index;
		string line = lines[line_index];
		int line_length = int(line.length());
		
		// 0 = whitespace
		// 1 = punctuation
		// 2 = alphanumeric
		// 3 = alphanumeric
		int chr_type = -1;
		
		while(true)
		{
			const int chr = index == line_length ? 10 : line[index];
			const int new_chr_type = chr == 10 || chr == 13 ? 3 : string::is_whitespace(chr) ? 0 : string::is_punctuation(chr) ? 1 : string::is_alphanumeric(chr) ? 2 : -1;
			
			if(chr_type == -1)
			{
				chr_type = new_chr_type;
			}
			else if(
				(new_chr_type == 3 || chr_type == 3) ||
				(chr_type == 2 && new_chr_type != 2) || (chr_type == 1 && new_chr_type != 1)
			)
			{
				break;
			}
				
			chr_type = new_chr_type;
			
			index += dir;
			
			if(index < 0)
			{
				if(line_index-- <= 0)
					break;
				
				line = lines[line_index];
				line_length = int(line.length());
				index = line_length;
			}
			else if(index > line_length)
			{
				if(++line_index >= _num_lines)
					break;
				
				line = lines[line_index];
				line_length = int(line.length());
				index = 0;
			}
		}
		
		if(dir == -1)
		{
			index++;
		}
		
		line_index = clamp(line_index, 0, num_lines - 1);
		
		return get_line_start_index(line_index) + clamp(index, 0, get_line_length(line_index) + 1);
	}
	
	/// Finds the next boundary starting from start_index in direction.
	/// If use_line_end and start_index is the end of the line, the last character in the line will be used instead.
	int expand_to_boundary(const int start_index, int dir, const bool use_line_end=false)
	{
		dir = dir >= 0 ? 1 : -1;
		
		if(start_index <= 0 && dir == -1)
			return 0;
		if(start_index >= _text_length && dir == 1)
			return _text_length;
		
		int line_index = get_line_at_index(start_index);
		const int line_end_index = line_end_indices[line_index];
		const int line_start_index = line_index > 0
			? line_end_indices[line_index - 1] + 1
			: 0;
		const int end = dir == 1 ? (line_end_index - line_start_index) : -1;
		int chr_index = start_index - line_start_index;
		string line = lines[line_index];
		
		// 0 = whitespace
		// 1 = punctuation
		// 2 = alphanumeric
		// 3 = newline
		int chr_type;
		int chr;
		
		// If we're at the end of the line, instead expand based on the last non newline character on this line
		if(start_index == line_end_index)
		{
			if(!use_line_end || chr_index == 0)
				return start_index;
			
			chr_index--;
		}
		
		chr = line[chr_index];
		chr_type = (chr == 10 || chr == 13) ? 3 : string::is_whitespace(chr) ? 0 : string::is_punctuation(chr) ? 1 : string::is_alphanumeric(chr) ? 2 : -1;
		
		while((chr_index += dir) != end)
		{
			chr = line[chr_index];
			const int new_chr_type = (chr == 10 || chr == 13) ? 3 : string::is_whitespace(chr) ? 0 : string::is_punctuation(chr) ? 1 : string::is_alphanumeric(chr) ? 2 : -1;
			
			if(new_chr_type != chr_type || new_chr_type == 3)
				break;
		}
		
		if(dir == -1)
		{
			chr_index++;
		}
		
		return clamp(line_start_index + chr_index, line_start_index, line_end_index);
	}
	
	/// Returns the relative position within this TextBox from the given global position,
	/// taking the current scroll position into account.
	void get_local_xy(const float global_x, const float global_y, float &out x, float &out y)
	{
		x = global_x - x1 - padding_left - _scroll_x;
		y = global_y - y1 - padding_top  - _scroll_y;
	}
	
	/// Returns the global position from the given text coordinates taking the current scroll position into account.
	void get_global_xy(const float local_x, const float local_y, float &out x, float &out y)
	{
		x = local_x + x1 + padding_left + _scroll_x;
		y = local_y + y1 + padding_top  + _scroll_y;
	}
	
	// ///////////////////////////////////////////////////////////////////
	// Line Utility
	// ///////////////////////////////////////////////////////////////////
	
	/// Returns the line at the specified character index.
	int get_line_at_index(int index)
	{
		if(index < 0)
			return 0;
		
		if(index >= _text_length)
			return max(0, _num_lines - 1);
		
		int left = 0;
		int right = _num_lines - 1;
		array<int>@ line_end_indices = @this.line_end_indices;
		
		while(right >= left)
		{
			const int mid = left + (right - left) / 2;
			const int value = line_end_indices[mid] + 1;
			
			if(value == index)
				return mid + 1;
			
			if(value < index)
			{
				left  = mid + 1;
			}
			else if(value > index)
			{
				right = mid - 1;
			}
		}
		
		if(right < 0)
			return 0;
		
		if(left > _num_lines - 1)
			return _num_lines - 1;
		
		return (left < right)
			? left  + 1
			: right + 1;
	}
	
	/// Returns the closest line index at the given y value.
	/// If relative is false, y is considered a global coordinate.
	int get_line_at_y(float y, const bool relative=true)
	{
		if(!relative)
		{
			y = y - y1 - padding_top - _scroll_y;
		}
		
		return clamp(floor_int((y + _line_spacing * 0.25) / (unscaled_line_height * _text_scale + _line_spacing)), 0, _num_lines - 1);
	}
	
	/// Returns the starting character index of the given line.
	/// Will return -1 if line_index is not valid.
	int get_line_start_index(const int line_index)
	{
		if(line_index < 0 || line_index >= _num_lines)
			return -1;
		
		return line_index > 0
			? line_end_indices[line_index - 1] + 1
			: 0;
	}
	
	/// Returns the ending character index of the given line, exlcuding the newline character
	/// Will return -1 if line_index is not valid.
	int get_line_end_index(const int line_index)
	{
		if(line_index < 0 || line_index >= _num_lines)
			return -1;
		
		return line_end_indices[line_index];
	}
	
	/// Returns the number of characters in this line, exluding the newline character
	/// Will return -1 if line_index is not valid.
	int get_line_length(const int line_index)
	{
		if(line_index < 0 || line_index >= _num_lines)
			return -1;
		
		return line_end_indices[line_index] - (line_index == 0 ? 0 : line_end_indices[line_index - 1] + 1);
	}
	
	/// Returns the width of the line. Will return 0 if line_index is not valid.
	float get_line_width(const int line_index)
	{
		if(line_index < 0 || line_index >= _num_lines)
			return 0;
		
		const int line_length = line_end_indices[line_index] - (line_index == 0 ? 0 : line_end_indices[line_index - 1] + 1);
		return line_length > 0
			? line_character_widths[line_index][line_length - 1]
			: 0;
	}
	
	/// Returns the character index relative to the line containing it.
	/// Returns -1 if index is not valid.
	int get_index_in_line(const int index)
	{
		const int line_index = get_line_at_index(index);
		
		if(line_index == -1)
			return -1;
		
		return index - (line_index > 0
			? line_end_indices[line_index - 1] + 1
			: 0);
	}
	
	/// Returns the relative y position of the given line index.
	float get_line_y(const int line_index)
	{
		return line_index >= 0
			? (unscaled_line_height * text_scale + _line_spacing) * (line_index >= _num_lines ? _num_lines - 1 : line_index)
			: 0;
	}
	
	// ///////////////////////////////////////////////////////////////////
	// Character Utility
	// ///////////////////////////////////////////////////////////////////
	
	/// Returns the closest line index at the given y value.
	/// If closest_boundary is false the closest index of the chracter at x,y is returned,
	/// otherwise the index/boundary boundary between two characters is returned.
	/// If relative is false, x and y are considered global coordinates.
	int get_index_at(float x, float y, const bool closest_boundary=false, const bool relative=true)
	{
		if(!relative)
		{
			get_local_xy(x, y, x, y);
		}
		
		const int line_index = get_line_at_y(y);
		const int line_start = get_line_start_index(line_index);
		
		if(x <= 0)
			return line_start;
		
		const int line_length = get_line_end_index(line_index) - line_start;
		const array<float>@ character_widths = @line_character_widths[line_index];
		
		int left = 0;
		int right = line_length - 1;
		
		while(right >= left)
		{
			const int mid = left + (right - left) / 2;
			
			if(character_widths[mid] < x)
			{
				left  = mid + 1;
			}
			else if(character_widths[mid] > x)
			{
				right = mid - 1;
			}
		}
		
		if(closest_boundary && line_length > 0 && left < line_length && x >= ((left == 0 ? 0 : character_widths[left - 1]) + character_widths[left]) * 0.5)
		{
			left++;
		}
		
		return line_start + left;
	}
	
	/// Returns the x position of the given index.
	float get_index_x(const int index)
	{
		if(_num_lines == 0 || index < 0)
			return 0;
		
		const int line_index = get_line_at_index(index);
		const int chr_index  = index - get_line_start_index(line_index);
		
		return chr_index == 0 ? 0 : line_character_widths[line_index][chr_index - 1];
	}
	
	/// Calculates the relative x and y position at the given index
	void get_index_xy(const int index, float &out x, float &out y)
	{
		if(_num_lines == 0 || index < 0)
		{
			x = 0;
			y = 0;
			return;
		}
		
		const int line_index = get_line_at_index(index);
		const int chr_index  = index - get_line_start_index(line_index);
		
		x = chr_index == 0 ? 0 : line_character_widths[line_index][chr_index - 1];
		y = (unscaled_line_height * text_scale + _line_spacing) * line_index;
	}
	
	// ///////////////////////////////////////////////////////////////////
	// Element
	// ///////////////////////////////////////////////////////////////////
	
	bool ui_step() override
	{
		if(busy_drag_scroll)
		{
			if(ui.mouse.secondary_down)
			{
				scroll_x = drag_scroll_start_x + ui.mouse.x - drag_mouse_x_start;
				scroll_y = drag_scroll_start_y + ui.mouse.y - drag_mouse_y_start;
			}
			else
			{
				busy_drag_scroll = false;
			}
		}
		
		if(drag_selection)
		{
			if(ui.mouse.moved || scrolled)
			{
				if(double_click_start_index == -1)
				{
					do_drag_selection();
				}
				else
				{
					do_boundary_drag_selection();
				}
				
				scrolled = false;
			}
		}
		
		if(persist_caret_time > 0)
		{
			persist_caret_time--;
		}
		
		return focused;
	}
	
	void _do_layout(LayoutContext@ ctx) override
	{
		update_scroll_values();
	}
	
	void _draw(Style@ style, DrawingContext@ ctx) override
	{
		////////////////////////////////////////
		// Bakground
		
		const uint border_clr = style.get_interactive_element_border_colour(hovered, focused, focused, disabled);
		
		const uint bg_clr = style.get_interactive_element_background_colour(false, false, false, disabled, true);
		const float inset = border_clr != 0 ? max(0.0, style.border_size) : 0;
		
		// Fill
		style.draw_rectangle(
			x1 + inset, y1 + inset,
			x2 - inset, y2 - inset,
			0, bg_clr);
		
		// Border
		const float border_size = focused ? style.selected_border_size : style.border_size;
		
		if(border_clr != 0 && border_size > 0)
		{
			style.outline(
				x1, y1,
				x2, y2,
				border_size, border_clr);
		}
		
		////////////////////////////////////////
		// Setup
		
		const float view_width  = _width  - padding_left - padding_right;
		const float view_height = _height - padding_top - padding_bottom;
		const float scroll_x = _scroll_x;
		const float scroll_y = _scroll_y;
		const float text_scale = _text_scale;
		const float text_width = _text_width;
		const float text_height = _text_height;
		const float line_height = unscaled_line_height * text_scale;
		const float line_spacing = line_height + _line_spacing;
		const int selection_start = _selection_start;
		const int selection_end = _selection_end;
		const int selection_start_line = get_line_at_index(selection_start);
		const int selection_end_line   = get_line_at_index(selection_end);
		const int first_visible_line = ceil_int(-scroll_y / line_spacing);
		const int last_visible_line  = min(_num_lines - 1, floor_int((-scroll_y + view_height + EPSILON - line_height) / line_spacing));
		
		float dx, dy;
		canvas@ c;
		textfield@ text_field = style._initialise_text_field(
			@c,
			dx, dy,
			style.text_clr,
			text_scale, text_scale, 0,
			TextAlign::Left, TextAlign::Top,
			_font, _size);
		
		const float x1 = this.x1;
		const float y1 = this.y1 + (!_multi_line ? (view_height - line_height) * 0.5 : 0);
		const float x = x1 + padding_left + scroll_x;
		const float y = y1 + padding_top + scroll_y + first_visible_line * (line_height + _line_spacing);
		float line_y;
		
		////////////////////////////////////////
		// Selection
		
		if(selection_start != selection_end)
		{
			const int selection_min_index = min(selection_start, selection_end);
			const int selection_max_index = max(selection_start, selection_end);
			const int selection_max_line_real = max(selection_start_line, selection_end_line);
			const int selection_min_line = max(first_visible_line, min(selection_start_line, selection_end_line));
			const int selection_max_line = min(last_visible_line, selection_max_line_real);
			const uint select_clr = focused ? style.secondary_bg_clr : multiply_alpha(style.secondary_bg_clr, 0.5);
			
			line_y = y + (selection_min_line - first_visible_line) * line_spacing;
			
			for(int line_index = selection_min_line; line_index <= selection_max_line; line_index++)
			{
				const int line_start_index = line_index == 0 ? 0 : line_end_indices[line_index - 1] + 1;
				const int line_end_index = line_end_indices[line_index];
				const int line_length = line_end_index - line_start_index;
				
				const array<float>@ character_widths = @line_character_widths[line_index];
				const int first_index = max(0, selection_min_index - line_start_index);
				const int last_index = max(0, min(line_length, selection_max_index - line_start_index));
				
				const float selection_x1 = scroll_x + (first_index == 0 ? 0 : character_widths[first_index - 1]);
				// Add some extra space to show that the newline is also selected
				const float selection_x2 = scroll_x + (last_index == 0 ? 0 : character_widths[last_index - 1]) + (line_index < selection_max_line_real ? 6 : 0);
				
				if(selection_x1 < view_width && selection_x2 > 0)
				{
					style.draw_rectangle(
						x1 + padding_left + max(selection_x1, 0.0),
						line_y - style.selection_padding_top,
						x1 + padding_left + min(selection_x2, view_width),
						line_y + line_height + style.selection_padding_bottom,
						0, select_clr);
				}
				
				line_y += line_spacing;
			}
		}
		
		////////////////////////////////////////
		// Text
		
		line_y = y + dy;
		
		for(int line_index = first_visible_line; line_index <= last_visible_line; line_index++)
		{
			const int line_length = line_end_indices[line_index] - (line_index == 0 ? 0 : line_end_indices[line_index - 1] + 1);
			
			if(line_length <= 0)
			{
				line_y += line_spacing;
				continue;
			}
			
			const array<float>@ character_widths = @line_character_widths[line_index];
			
			////////////////////////////////
			// Binary search to find the first visible character
			
			int left = 0;
			int right = line_length - 1;
			
			while(left <= right)
			{
				const int mid = left + (right - left) / 2;
				const float chr_left = scroll_x + (mid == 0 ? 0 : character_widths[mid - 1]);
				
				if(chr_left == 0)
				{
					left = mid;
					break;
				}
				
				if(chr_left < 0)
					left = mid + 1;
				else
					right = mid - 1;
			}
			
			const int start_chr_index = left;
			
			////////////////////////////////
			// Binary search to find the last visible character
			
			right = line_length - 1;
			
			while(left <= right)
			{
				const int mid = left + (right - left) / 2;
				const float chr_right = scroll_x + character_widths[mid];
				
				if(chr_right == view_width)
				{
					right = mid;
					break;
				}
				
				if(chr_right > view_width)
					right = mid - 1;
				else
					left = mid + 1;
			}
			
			const int end_chr_index = right + 1;
			
			/////////////////////////////////////////////////////////
			// Draw in chunks of 64 characters since that seems to be the limit for that textfield can draw
			
			const int draw_count = end_chr_index - start_chr_index;
			
			if(start_chr_index != end_chr_index)
			{
				const int chunk_size = 32;
				float chunk_x = x + dx + (start_chr_index == 0 ? 0 : character_widths[start_chr_index - 1]);
				
				int characters_drawn = start_chr_index;
				const bool draw_in_chunks = draw_count != line_length || line_length > chunk_size;
				
				while(characters_drawn < end_chr_index)
				{
					text_field.text((draw_in_chunks
						? lines[line_index].substr(characters_drawn, min(end_chr_index - characters_drawn, chunk_size))
						: lines[line_index]));
					c.draw_text(text_field, chunk_x, line_y, text_scale, text_scale, 0);
					
					if(!draw_in_chunks)
						break;
					
					chunk_x += text_field.text_width() * text_scale;
					characters_drawn += chunk_size;
				}
			}
			
			line_y += line_spacing;
		}
		
		////////////////////////////////////////
		// Caret
		
		if(
			focused &&
			selection_end_line >= first_visible_line && selection_end_line <= last_visible_line &&
			(persist_caret_time > 0 || ((ui._frame % ui.style.caret_blink_rate) > ui.style.caret_blink_rate / 2))
		)
		{
			const int chr_index = selection_end - get_line_start_index(selection_end_line);
			const float caret_x = scroll_x + (chr_index == 0 ? 0 : line_character_widths[selection_end_line][chr_index - 1]);
			const float caret_y = y + (selection_end_line - first_visible_line) * line_spacing;
			
			if(caret_x >= 0 && caret_x <= view_width)
			{
				style.draw_rectangle(
					caret_x + x1 + padding_left - style.caret_width * 0.5,
					caret_y - style.selection_padding_top,
					caret_x + x1 + padding_left + style.caret_width * 0.5,
					caret_y + line_height + style.selection_padding_bottom,
					0, style.selected_highlight_border_clr);
			}
		}
		
		// Debug
		
//		line_y = y1 + dy + padding_top + scroll_y;
//		
//		for(int line_index = 0; line_index < _num_lines; line_index++)
//		{
//			style.outline(x - dx, line_y - dy, x - dx + get_line_width(line_index), line_y - dy + line_height, 1, 0x77ff0000);
//			line_y += line_spacing;
//		}
//		
//		style.outline(
//			x1 + _scroll_x + padding_left, y1 + _scroll_y + padding_top,
//			x1 + _scroll_x + padding_left + text_width, y1 + _scroll_y + padding_top + text_height,
//			1, 0x9900ffff);
	}
	
	// ///////////////////////////////////////////////////////////////////
	// Private
	// ///////////////////////////////////////////////////////////////////
	
	/// Returns the number of chracters removed as the low 32 bits and the number inserted as the high 32 bits.
	protected int64 do_replace(int start_index, int end_index, const string text)
	{
		////////////////////////////////////
		// Setup.
		
		start_index = clamp(start_index, 0, _text_length);
		end_index   = clamp(end_index, 0, _text_length);
		
		if(end_index < start_index)
		{
			const int start_index_t = start_index;
			start_index = end_index;
			end_index = start_index_t;
		}
		
		const auto@ font_metrics		 = @this.font_metrics;
		const float text_scale			 = _text_scale;
		const int first_valid_char_index = ui.first_valid_char_index;
		const int last_valid_char_index  = ui.last_valid_char_index;
		
		const int insert_text_length	= int(text.length());
		const int start_line_index		= get_line_at_index(start_index);
		const int end_line_index		= get_line_at_index(end_index);
		const int remove_count			= end_index - start_index;
		
		// puts('== do_replace================================================');
		// puts('=============================================================');
		// puts('  with: "' + text + '"');
		// puts('  range: ' + start_index + ' > ' + end_index);
		// puts('  start_line_index: ' + start_line_index);
		// puts('  end_line_index: ' + end_line_index);
		// puts('  remove_count: ' + remove_count);
		
		int current_line_index			= start_line_index;
		int current_line_start_index	= current_line_index > 0 ? line_end_indices[current_line_index - 1] + 1 : 0;
		int current_line_end_index		= line_end_indices[current_line_index];
		array<float>@ character_widths	= @line_character_widths[current_line_index];
		int character_widths_size		= int(character_widths.length());
		int chr_index					= start_index - current_line_start_index;
		float current_line_width		= chr_index > 0 ? character_widths[chr_index - 1] : 0;
		string line_buffer				= lines[current_line_index].substr(0, chr_index);
		int line_buffer_size			= int(lines[current_line_index].length()) + 32;
		
		line_buffer.resize(line_buffer_size);
		
		// Get the contents of the the last line after end_index
		// so that it can be merged with the start line.
		const string end_line_text		= lines[end_line_index].substr(end_index - get_line_start_index(end_line_index));
		
		// puts('  chr_index: ' + chr_index);
		// puts('  line_buffer: "' + line_buffer + '"');
		// puts('  current_line_width: ' + current_line_width);
		// puts('  end_line_text: "' + end_line_text + '"');
		
		int i = 0;
		int insert_count = 0;
		float text_width = 0;
		
		//////////////////////////////////////////////////////////////////////
		// Validate and insert new text, overwriting the removed text
		// and the adding new lines when necessary
		
		while(true)
		{
			int chr = i < insert_text_length ? int(text[i++]) : -1;
			
			if(chr == 13)
				chr = 10;
			if((chr == 10 || chr == 13) && !_multi_line)
				chr = -1;
			
			///////////////////////////////////////////
			// Push the buffered text to this line and then start a new line
			if(chr == 10 || chr == -1) // \n or end of text
			{
				if(chr != -1)
				{
					insert_count++;
				}
				
				if(current_line_index <= end_line_index)
				{
					lines[current_line_index]			 	= line_buffer.substr(0, chr_index);
					line_end_indices[current_line_index]	= current_line_start_index + chr_index;
				}
				else
				{
					lines.insertAt			 (current_line_index, line_buffer.substr(0, chr_index));
					line_end_indices.insertAt(current_line_index, current_line_start_index + chr_index);
				}
				
				if(current_line_width > text_width)
				{
					text_width = current_line_width;
				}
				
				// Reset
				
				if(chr == -1)
					break;
				
				current_line_start_index += chr_index + 1;
				chr_index = 0;
				current_line_width = 0;
				current_line_index++;
				
				if(current_line_index <= end_line_index)
				{
					@character_widths = @line_character_widths[current_line_index];
					character_widths_size = int(character_widths.length());
				}
				else
				{
					@character_widths = array<float>();
					line_character_widths.insertAt(current_line_index, @character_widths);
					character_widths_size = 0;
				}
				
				continue;
			} // END IF NEW LINE
			
			///////////////////////////////////////////
			// TODO: Check if chr is valid based on settings
			
			
			///////////////////////////////////////////
			// Append char
			//{
				const float chr_width = chr <= last_valid_char_index && chr >= first_valid_char_index
					? font_metrics[chr - first_valid_char_index] * text_scale
					: 0;
				
				if(chr_index >= line_buffer_size)
				{
					line_buffer.resize(line_buffer_size += 32);
				}
				
				if(chr_index >= character_widths_size)
				{
					character_widths.resize(character_widths_size += 32);
					character_widths[8] = (current_line_index);
				}
				
				current_line_width += chr_width;
				line_buffer[chr_index] = chr;
				character_widths[chr_index] = current_line_width;
				
				insert_count++;
				chr_index++;
			//}
		}
		
		if(insert_count == 0 && remove_count == 0)
			return 0;
		
		////////////////////////////////////////
		// Merge end line
		
		// puts('  insert_count: ' + insert_count);
		
		const int end_line_text_length			= int(end_line_text.length());
		
		// puts('  MERGING LINE: ' + current_line_index);
		// puts('  end index: ' + line_end_indices[current_line_index] + ' + ' + end_line_text_length);
		
		lines[current_line_index]				= lines[current_line_index] + end_line_text;
		line_end_indices[current_line_index]	+= end_line_text_length;
		
		if(chr_index + end_line_text_length > character_widths_size)
		{
			character_widths.resize(character_widths_size = chr_index + end_line_text_length + 32);
		}
		
		for(i = 0; i < end_line_text_length; i++)
		{
			int chr = int(end_line_text[i]);
			const float chr_width = chr <= last_valid_char_index && chr >= first_valid_char_index
					? font_metrics[chr - first_valid_char_index] * text_scale
					: 0;
			
			current_line_width += chr_width;
			character_widths[chr_index + i] = current_line_width;
			
			if(current_line_width > text_width)
			{
				text_width = current_line_width;
			}
		}
		
		////////////////////////////////////////
		// Remove deleted lines
		
		const int line_count_different = end_line_index - current_line_index;
		_num_lines -= line_count_different;
		current_line_index++;
		
		// puts('  line_difference: ' + line_count_different);
		
		if(line_count_different > 0)
		{
			// removeRange seems to be broken so manually remove
			for(int j = current_line_index; j < _num_lines; j++)
			{
				lines[j]					= lines[j + line_count_different];
				@line_character_widths[j]	= @line_character_widths[j + line_count_different];
				line_end_indices[j]			= line_end_indices[j + line_count_different];
			}
			
			 lines.resize(_num_lines);
			 line_character_widths.resize(_num_lines);
			 line_end_indices.resize(_num_lines);
			
			// lines.removeRange(current_line_index, line_count_different);
			// line_character_widths.removeRange(current_line_index, line_count_different);
			// line_end_indices.removeRange(current_line_index, line_count_different);
		}
		
		////////////////////////////////////
		// Recalculate text width and height, and adjust line endings
		
		const int length_difference = insert_count - remove_count;
		
		// Calculate text_width from all lines before the insertion
		for(int j = 0; j < start_line_index; j++)
		{
			const int before_line_end = line_end_indices[j];
			const int before_line_length = before_line_end - (j > 0 ? (line_end_indices[j - 1] + 1) : 0);
			
			if(before_line_length <= 0)
				continue;
			
			const float before_line_width = line_character_widths[j][before_line_length - 1];
			
			if(before_line_width > text_width)
			{
				text_width = before_line_width;
			}
		}
		
		// Update all line end indices from this line onwards, and update text_width
		for(int j = current_line_index; j < _num_lines; j++)
		{
			const int after_line_end = line_end_indices[j] + length_difference;
			const int after_line_length = after_line_end - (j > 0 ? (line_end_indices[j - 1] + 1) : 0);
			line_end_indices[j] = after_line_end;
			
			if(after_line_length <= 0)
				continue;
			
			const float after_line_width = line_character_widths[j][after_line_length - 1];
			
			if(after_line_width > text_width)
			{
				text_width = after_line_width;
			}
		}
		
		_text_length += length_difference;
		_text_width = text_width;
		recalculate_text_height();
		
		// debug_lines();
		
		line_relative_caret_index = -1;
		
		dispatch_change_event();
		
		return (int64(insert_count) << 32) | remove_count;
	}
	
	/// Shifts the selection end points by shift_amount, but only if that end point is after start_index.
	/// Will also make sure both end points are valid
	protected void adjust_selection(const int start_index, const int shift_amount, const bool move_caret_to_end)
	{
		if(shift_amount != 0)
		{
			if(_selection_start >= start_index)
			{
				_selection_start = clamp(_selection_start + shift_amount, start_index, _text_length);
			}
			else
			{
				_selection_start = clamp(_selection_start, 0, _text_length);
			}
			
			if(_selection_end >= start_index)
			{
				_selection_end = clamp(_selection_end + shift_amount, start_index, _text_length);
			}
			else
			{
				_selection_end = clamp(_selection_end, 0, _text_length);
			}
		}
		
		if(move_caret_to_end)
		{
			caret_index = max(_selection_start, _selection_end);
		}
	}
	
	/// Makes sure the selection indices are within the text bounds
	protected void validate_selection()
	{
		_selection_start = clamp(_selection_start, 0, _text_length);
		_selection_end   = clamp(_selection_end,   0, _text_length);
	}
	
	/// This will need to rescan each character in each line
	protected void recalculate_text_width(const bool update_lines=true)
	{
		float text_width = 0;
		const int first_valid_char_index = ui.first_valid_char_index;
		const int last_valid_char_index  = ui.last_valid_char_index;
		
		for(int line_index = _num_lines - 1; line_index >= 0; line_index--)
		{
			const int line_length = line_end_indices[line_index] - (line_index == 0 ? 0 : line_end_indices[line_index - 1] + 1);
			
			if(line_length <= 0)
				continue;
			
			if(!update_lines)
			{
				if(line_character_widths[line_index][line_length - 1] > text_width)
				{
					text_width = line_character_widths[line_index][line_length - 1];
				}
				
				continue;
			}
			
			array<float>@ character_widths = @line_character_widths[line_index];
			const string line_text = lines[line_index];
			float line_width = 0;
			
			for(int chr_index = 0; chr_index < line_length; chr_index++)
			{
				const int chr = int(line_text[chr_index]);
				line_width += chr <= last_valid_char_index && chr >= first_valid_char_index
						? font_metrics[chr - first_valid_char_index] * text_scale
						: 0;
				character_widths[chr_index] = line_width;
			}
			
			if(line_width > text_width)
			{
				text_width = line_width;
			}
		}
		
		_text_width = text_width;
		
		update_scroll_values();
	}
	
	/// Simply multiply the number of lines with the line height
	protected void recalculate_text_height()
	{
		const int num_lines = max(1, _num_lines);
		_text_height = (unscaled_line_height * _text_scale) * num_lines + _line_spacing * (num_lines - 1);
		
		update_scroll_values();
	}
	
	protected void update_scroll_values()
	{
		const float view_width  = _width  - padding_left - padding_right;
		const float view_height = _height - padding_top - padding_bottom;
		
		scroll_max_x = max(0.0, _text_width  - view_width);
		scroll_max_y = max(0.0, _text_height - view_height);
		
		_scroll_x = clamp_scroll(_scroll_x, scroll_max_x);
		_scroll_y = clamp_scroll(_scroll_y, scroll_max_y);
	}
	
	protected float clamp_scroll(const float scroll, const float max)
	{
		if(scroll < -max)
			return -max;
		
		if(scroll > 0)
			return 0;
		
		return scroll;
	}
	
	protected void persist_caret()
	{
		persist_caret_time = ui.style.caret_blink_rate;
	}
	
	protected void debug_lines(const bool print_char_widths=true)
	{
		puts('== ' + id + ' ========================================');
		puts('   _text_length: ' + _text_length);
		puts('   _num_lines: ' + _num_lines);
		
		for(int i = 0; i < _num_lines; i++)
		{
			puts('  -------------------------');
			puts('  -- [' + i + '] "' + lines[i] + '"');
			puts('      end: ' + line_end_indices[i]);
			puts('      length: ' + get_line_length(i));
			puts('      width: ' + get_line_width(i));
			
			if(print_char_widths)
				puts('      widths: ' + string::join(@line_character_widths[i], ' '));
		}
		
		puts('=============================================================');
	}
	
	//
	
	protected void do_drag_selection(const bool extend_selection=true)
	{
		const int index = get_index_at(ui.mouse.x, ui.mouse.y, true, false);
		
		if(extend_selection)
		{
			selection_end = index;
		}
		else
		{
			caret_index = index;
		}
		
		try_scroll_to_mouse_or_caret();
		persist_caret();
	}
	
	protected void try_scroll_to_mouse_or_caret()
	{
		float x, y;
		get_local_xy(ui.mouse.x, ui.mouse.y, x, y);
		scroll_into_view(x, y, x, y);
		this.scroll_to_caret(0);
	}
	
	protected void start_boundary_drag_selection()
	{
		const int caret_index = get_index_at(ui.mouse.x, ui.mouse.y, false, false);
		
		double_click_start_index = expand_to_boundary(caret_index, -1, true);
		double_click_end_index   = expand_to_boundary(caret_index,  1, true);
		selection_start = double_click_start_index;
		selection_end   = double_click_end_index;
		
		drag_selection = true;
		this.scroll_to_caret(0);
		persist_caret();
	}
	
	protected void do_boundary_drag_selection()
	{
		const int caret_index = get_index_at(ui.mouse.x, ui.mouse.y, false, false);
		
		if(caret_index > double_click_start_index)
		{
			selection_start = double_click_start_index;
			selection_end = expand_to_boundary(caret_index,  1);
		}
		else
		{
			selection_start = double_click_end_index;
			selection_end = expand_to_boundary(caret_index,  -1);
		}
		
		try_scroll_to_mouse_or_caret();
		persist_caret();
	}
	
	/// Returns the selection end line index
	protected int update_relative_line_index()
	{
		const int line_index = get_line_at_index(_selection_end);
		
		if(line_relative_caret_index == -1)
		{
			line_relative_caret_index = _selection_end - get_line_start_index(line_index);
		}
		
		return line_index;
	}
	
	protected void dispatch_change_event(const bool changed=true)
	{
		if(suppress_change_event)
		{
			suppress_change_event = false;
			return;
		}
		
		if(!changed)
			return;
		
		ui._dispatch_event(@change, EventType::CHANGE, @this);
	}
	
	// ///////////////////////////////////////////////////////////////////
	// Events
	// ///////////////////////////////////////////////////////////////////
	
	void _mouse_press(EventInfo@ event)
	{
		const bool was_focused = focused;
		
		@ui.focus = @this;
		
		if(_select_all_on_focus && !was_focused)
			return;
		
		if(event.button == ui.primary_button)
		{
			if(ui.mouse.primary_double_click)
			{
				start_boundary_drag_selection();
			}
			else
			{
				do_drag_selection(ui._has_editor && ui._editor.key_check_gvb(GVB::Shift));
				drag_selection = true;
			}
		}
		else if(_drag_scroll && event.button == ui.secondary_button)
		{
			drag_mouse_x_start = ui.mouse.x;
			drag_mouse_y_start = ui.mouse.y;
			drag_scroll_start_x = _scroll_x;
			drag_scroll_start_y = _scroll_y;
			busy_drag_scroll = true;
		}
	}
	
	void _mouse_release(EventInfo@ event)
	{
		if(event.button == ui.primary_button)
		{
			busy_drag_scroll = false;
		}
		
		if(event.button != ui.primary_button)
			return;
		
		drag_selection = false;
		double_click_start_index = -1;
		double_click_end_index = -1;
	}
	
	void _mouse_scroll(EventInfo@ event)
	{
		if(ui._has_editor && ui._editor.key_check_gvb(GVB::Shift))
		{
			scroll_x -= event.mouse.scroll * (font_metrics[0] * 7);
		}
		else
		{
			scroll_y -= event.mouse.scroll * (unscaled_line_height * text_scale + _line_spacing);
		}
		
		scrolled = true;
	}
	
	void on_focus(Keyboard@ keyboard) override
	{
		FocusableElement::on_focus(keyboard);
		
		keyboard.register_arrows_gvb();
		keyboard.register_gvb(GVB::Delete, ModifierKey::Ctrl);
		keyboard.register_gvb(GVB::Back, ModifierKey::Ctrl);
		keyboard.register_range_vk(VK::End, VK::Home, ModifierKey::Ctrl | ModifierKey::Shift);
		keyboard.register_vk(VK::A, ModifierKey::Ctrl | ModifierKey::Only);
		keyboard.register_inputs();
		
		if(_remove_lines_shortcut)
		{
			keyboard.register_vk(VK::D, ModifierKey::Ctrl | ModifierKey::Shift | ModifierKey::Only);
		}
		
		if(_clipboard_enabled)
		{
			keyboard.register_vk(VK::C, ModifierKey::Ctrl | ModifierKey::Only);
			keyboard.register_vk(VK::X, ModifierKey::Ctrl | ModifierKey::Only);
			keyboard.register_vk(VK::V, ModifierKey::Ctrl | ModifierKey::Only);
		}
		
		if(_select_all_on_focus)
		{
			select_all();
		}
		
		if(_revert_on_cancel)
		{
			previous_text = text;
		}
		
		ui._step_subscribe(this);
	}
	
	void on_blur(Keyboard@ keyboard, const BlurAction type) override
	{
		FocusableElement::on_blur(keyboard, type);
		
		if(
			type == BlurAction::Accepted ||
			type == BlurAction::None && _accept_on_blur ||
			type == BlurAction::Cancelled && !_revert_on_cancel)
		{
			ui._dispatch_event(@accept, EventType::ACCEPT, @this);
		}
		else if(type == BlurAction::Cancelled && _revert_on_cancel)
		{
			suppress_change_event = true;
			text = previous_text;
			ui._dispatch_event(@accept, EventType::CANCEL, @this);
		}
		
		if(_deselect_all_on_blur)
		{
			select_none();
		}
	}
	
	void on_key_press(Keyboard@ keyboard, const int key, const bool is_gvb, const string text)
	{
		if(!is_gvb && key == VK::A && keyboard.ctrl)
		{
			select_all();
			return;
		}
		
		on_key(keyboard, key, is_gvb, text);
	}
	
	void on_key(Keyboard@ keyboard, const int key, const bool is_gvb, const string text)
	{
		if(!is_gvb)
		{
			if(key == VK::D && keyboard.ctrl && keyboard.shift)
			{
				remove_lines(get_line_at_index(_selection_start), get_line_at_index(_selection_end), 8);
				return;
			}
			
			if(key == VK::C && keyboard.ctrl)
			{
				copy();
				return;
			}
			
			if(key == VK::X && keyboard.ctrl)
			{
				cut(8);
				return;
			}
			
			if(key == VK::V && keyboard.ctrl)
			{
				paste(8);
				return;
			}
		}
		
		if(is_gvb)
		{
			switch(key)
			{
				case GVB::LeftArrow:
					move_caret_left(keyboard.ctrl, keyboard.shift, true);
					break;
				case GVB::RightArrow:
					move_caret_right(keyboard.ctrl, keyboard.shift, true);
					break;
				case GVB::UpArrow:
					move_caret_up(keyboard.shift, true);
					break;
				case GVB::DownArrow:
					move_caret_down(keyboard.shift, true);
					break;
				case GVB::Delete:
					delete(keyboard.ctrl, 1, 8);
					persist_caret();
					break;
				case GVB::Back:
					delete(keyboard.ctrl, -1, 8);
					persist_caret();
					break;
			}
			
			return;
		}
		
		if(text != '')
		{
			replace(text, true, 8);
			persist_caret();
			return;
		}
		
		switch(key)
		{
			case VK::Home:
				move_caret_home(ui.keyboard.ctrl, ui.keyboard.shift, true);
				break;
			case VK::End:
				move_caret_end(ui.keyboard.ctrl, ui.keyboard.shift, true);
				break;
		}
	}
	
}
