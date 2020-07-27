#include '../std.cpp';
#include '../string.cpp';
#include '../fonts.cpp';
#include '../math/math.cpp';
#include '../utils/colour.cpp';
#include 'UIMouse.cpp';
#include 'Style.cpp';
#include 'utils/BoolStack.cpp';
#include 'utils/ElementStack.cpp';
#include 'utils/pools/ImagePool.cpp';
#include 'utils/pools/LabelPool.cpp';
#include 'events/Event.cpp';
#include 'events/GenericEvent.cpp';
#include 'elements/Element.cpp';
#include 'elements/Container.cpp';
#include 'elements/Tooltip.cpp';
#include 'layouts/flow/FlowLayout.cpp';

class UI
{
	
	int NEXT_ID;
	int CUSTOM_TOOLTIP_ID;
	
	// Which mouse button is primarily used to interact with UI elements.
	// Left might be more problematic since it will also interact with the editor ui.
	MouseButton primary_button = MouseButton::Right;
	
	Style@ style;
	UIMouse@ mouse;
	bool is_mouse_over;
	
	// Uncomment and manually set during testing
	Debug@ debug;
	
	private scene@ g;
	
	private bool _hud;
	private int _layer;
	private int _sub_layer;
	private int _player;
	
	private Container@ contents;
	private Container@ overlays;
	// e.g. a drop down box that is open. There can only by one active element in a UI.
	private Element@ active_element;
	
	// Used for processing element layouts
	private ElementStack element_stack;
	// Used for processing element layouts
	private BoolStack mouse_stack;
	// The top most element the mouse is over
	private Element@ _mouse_over_element;
	// Elements entered on this frame
	private array<Element@> elements_mouse_enter();
	// The hierarchy of elements the mouse is over, from the outermost to the inner
	private array<Element@> elements_mouse_over();
	
	private Element@ mouse_over_overlays;
	
	private dictionary elements_left_pressed();
	private dictionary elements_right_pressed();
	private dictionary elements_middle_pressed();
	private array<Element@> elements_pressed_list();
	
	private dictionary tooltips;
	
	private textfield@ debug_text_field;
	
	private float x1 = 0;
	private float y1 = 0;
	private float x2 = 100;
	private float y2 = 100;
	
	private EventCallback@ on_tooltip_hide_delegate;
	
	// ///////////////////////////////////////////////////////////////
	// Common reusable things
	
	/*private*/ LabelPool _label_pool;
	/*private*/ ImagePool _image_pool;
	
	/*private*/ EventInfo@ _event_info = EventInfo();
	/*private*/ GenericEventInfo@ _generic_event_info = GenericEventInfo();
	/*private*/ FlowLayout@ _toolbar_flow_layout;
	
	/*private*/ array<float> _float_array(16);
	/*private*/ array<int> _int_array(16);
	/*private*/ array<Element@> _element_array(16);
	
	UI(bool hud=true, int layer=20, int sub_layer=19, int player=0)
	{
		@on_tooltip_hide_delegate = EventCallback(this.on_tooltip_hide);
		
		@contents = Container(this);
		@overlays = Container(this);
		contents._id = '_ROOT_';
		overlays._id = '_OVERLAYS_';
		
		_hud = hud;
		_layer = layer;
		_sub_layer = sub_layer;
		_player = player;
		
		if(hud)
		{
			contents.x = SCREEN_LEFT;
			contents.y = SCREEN_TOP;
			contents.width  = SCREEN_RIGHT - SCREEN_LEFT;
			contents.height = SCREEN_BOTTOM - SCREEN_TOP;
		}
		
		@g = get_scene();
		@mouse = UIMouse(hud, layer, player);
		
		@_event_info.mouse = mouse;
		
		@style = Style(hud);
		
		@debug_text_field = create_textfield();
		debug_text_field.set_font(font::PROXIMANOVA_REG, 26);
		
		set_region(x1, y1, x2, y2);
		
		@_toolbar_flow_layout = FlowLayout(this, FlowDirection::Row, FlowAlign::Start, FlowAlign::Centre, FlowWrap::Wrap);
	}
	
	// The top most element the mouse is over
	Element@ mouse_over_element { get { return @_mouse_over_element; } }
	
	/**
	 * @brief Returns mouse x relative to this element
	 */
	float mouse_x { get { return mouse.x - contents.x1; } }
	
	/**
	 * @brief Returns mouse y relative to this element
	 */
	float mouse_y { get { return mouse.y - contents.y1; } }
	
	bool hud
	{
		get { return _hud; }
		set { mouse.hud = style._hud = _hud = value; }
	}
	
	uint layer
	{
		get { return _layer; }
		set { mouse.layer = style._layer = _layer = value; }
	}
	
	uint sub_layer
	{
		get { return _sub_layer; }
		set { style._sub_layer = _sub_layer = value; }
	}
	
	bool add_child(Element@ child)
	{
		return contents.add_child(child);
	}
	
	bool remove_child(Element@ child)
	{
		return contents.remove_child(child);
	}
	
	void move_to_front(Element@ child)
	{
		contents.move_to_front(child);
	}
	
	void move_to_back(Element@ child)
	{
		contents.move_to_back(child);
	}
	
	void move_up(Element@ child)
	{
		contents.move_up(child);
	}
	
	void move_down(Element@ child)
	{
		contents.move_down(child);
	}
	
	void clear()
	{
		contents.clear();
	}
	
	void step()
	{
		process_mouse_events(@_mouse_over_element == @mouse_over_overlays ? overlays : contents);
		
		mouse.step();
		
		switch(primary_button)
		{
			case MouseButton::Left:
				mouse.primary_down = mouse.left_down;
				mouse.primary_press = mouse.left_press;
				mouse.primary_release = mouse.left_release;
				break;
			case MouseButton::Middle:
				mouse.primary_down = mouse.middle_down;
				mouse.primary_press = mouse.middle_press;
				mouse.primary_release = mouse.middle_release;
				break;
			case MouseButton::Right:
				mouse.primary_down = mouse.right_down;
				mouse.primary_press = mouse.right_press;
				mouse.primary_release = mouse.right_release;
				break;
		}
		
		/*
		 * Update layout
		 */
		
		@_mouse_over_element = null;
		
		const bool mouse_in_ui = mouse.x >= contents.x1 && mouse.x <= contents.x2 && mouse.y >= contents.y1 && mouse.y <= contents.y2;
		
		Element@ mouse_over_main = update_layout(contents, mouse_in_ui);
		@_mouse_over_element = mouse_over_main;
		
		@mouse_over_overlays = update_layout(overlays, mouse_in_ui);
		
		if(@mouse_over_overlays != null)
		{
			@_mouse_over_element = @mouse_over_overlays;
		}
		
		// Process the mouse events before the start of the next frame.
		// Changes made during an event callback might affect the layout so when draw is called next
		// some elements might not be positioned correctly.
		// Wait for the next frame so that these changes will be reflected when the next layout pass happens.
	}
	
	void draw(float sub_frame)
	{
		style._layer = _layer;
		style._sub_layer = _sub_layer;
		
		contents._draw(@style);
		style._reset_state();
		
		overlays._draw(@style);
		style._reset_state();
	}
	
	void debug_draw(bool just_outline=false, const float id_scale=0.4)
	{
		style.outline(contents.x1, contents.y1, contents.x2, contents.y2, -2, 0xaaffffff);
		
		if(just_outline)
			return;
		
		element_stack.clear();
		contents._queue_children_for_layout(@element_stack);
		Element@ element = element_stack.pop();
		
		debug_text_field.align_horizontal(-1);
		debug_text_field.align_vertical(-1);
		const uint alpha = 0x55000000;
		uint clr;
		
		const float view_x1 = contents.x1;
		const float view_y1 = contents.y1;
		const float view_x2 = contents.x2;
		const float view_y2 = contents.y2;
		
		while(@element != null)
		{
			if(element.visible && @element != @mouse_over_element)
			{
				clr = get_element_id_colour(element, 0x00);
				
				if(element.x1 <= view_x2 && element.x2 >= view_x1 && element.y1 <= view_y2 && element.y2 >= view_y1)
				{
					style.outline(
						element.x1, element.y1, element.x2, element.y2,
						1, (element.hovered ? 0xff0000 : clr) | alpha);
					
					clr = scale_lightness(clr | alpha, 0.1);
				}
				else
				{
					style.outline_dotted(element.x1, element.y1, element.x2, element.y2, 1, clr | alpha);
					clr = scale_lightness(clr, 0.1) | alpha;
				}
				
				style.draw_text(
					element._id,
					element.x1, element.y1,
					clr | 0xff000000,
					id_scale, id_scale);
			}
			
			element._queue_children_for_layout(@element_stack);
			@element = element_stack.pop();
		}
		
		if(@mouse_over_element != null)
		{
			clr = get_element_id_colour(mouse_over_element);
			
			style.outline(mouse_over_element.x1, mouse_over_element.y1, mouse_over_element.x2, mouse_over_element.y2, -2, 0xffffffff);
			
			style.outline_text(
				mouse_over_element._id,
				mouse_over_element.x1, mouse_over_element.y1,
				0xffffffff, 0xff000000, 2 * id_scale,
				id_scale, id_scale);
			
			// Debug print mouse stack
			if(@debug != null)
			{
				const int num_elements_mouse_over = int(elements_mouse_over.size());
				
				for(int i = num_elements_mouse_over - 1; i >= 0; i--)
				{
					@element = @elements_mouse_over[i];
					debug.print(string::repeat('- ', i) + element._id, set_alpha(get_element_id_colour(element), 1), element._id, 1);
				}
			}
		}
	}
	
	void set_region(const float x1, const float y1, const float x2, const float y2)
	{
		this.x1 = x1;
		this.y1 = y1;
		this.x2 = x2;
		this.y2 = y2;
		
		contents.x = x1;
		contents.y = y1;
		contents.width  = x2 - x1;
		contents.height = y2 - y1;
		
		overlays.x = x1;
		overlays.y = y1;
		overlays.width  = x2 - x1;
		overlays.height = y2 - y1;
	}
	
	void get_region(float &out x1, float &out y1, float &out x2, float &out y2)
	{
		x1 = this.x1;
		y1 = this.y1;
		x2 = this.x2;
		y2 = this.y2;
	}
	
	float region_x1 { get const{ return x1; } }
	float region_y1 { get const{ return y1; } }
	float region_x2 { get const{ return x2; } }
	float region_y2 { get const{ return y2; } }
	float region_width  { get const{ return x2 - x1; } }
	float region_height { get const{ return y2 - y1; } }
	
	/**
	 * @brief Shows the tooltip for the given element if it has one.
	 * @param wait_for_mouse - If true and the tooltip hide type is MouseLeave, the tooltip will not close until the mouse enters it for the first time.
	 */
	void show_tooltip(Element@ element, bool wait_for_mouse=false)
	{
		if(@element == null || @element.tooltip == null)
			return;
		
		show_tooltip(element._id, element.tooltip, element, wait_for_mouse);
	}
	
	void hide_tooltip(Element@ element)
	{
		if(@element == null || @element.tooltip == null)
			return;
		
		hide_tooltip(element._id);
	}
	
	/**
	 * @param wait_for_mouse - If true and the tooltip hide type is MouseLeave, the tooltip will not close until the mouse enters it for the first time.
	 */
	void show_tooltip(TooltipOptions@ options, bool wait_for_mouse=false)
	{
		if(@options == null)
			return;
		
		if(options._id == '')
			options._id = '_tt_target' + (CUSTOM_TOOLTIP_ID++);
		
		show_tooltip(options._id, options, null, wait_for_mouse);
	}
	
	void hide_tooltip(TooltipOptions@ options)
	{
		if(@options == null)
			return;
		
		hide_tooltip(options._id);
	}
	
	void update_tooltip(Element@ element)
	{
		if(@element == null || @element.tooltip == null)
			return;
		
		update_tooltip(element._id, element.tooltip);
	}
	
	void update_tooltip(TooltipOptions@ options)
	{
		if(@options == null)
			return;
		
		update_tooltip(options._id, options);
	}
	
	// Private
	// ---------------------------------------------------------
	
	private Element@ update_layout(Element@ base, bool check_mouse_over)
	{
		Element@ mouse_over = null;
		
		ElementStack@ element_stack = @this.element_stack;
		BoolStack@ mouse_stack = @this.mouse_stack;
		
		element_stack.clear();
		mouse_stack.clear();
		
		base._do_layout();
		base._queue_children_for_layout(@element_stack);
		base.update_world_bounds();
		
		int stack_size = element_stack.size;
		
		if(check_mouse_over)
		{
			for(int i = 0; i < stack_size; i++)
			{
				mouse_stack.push(true);
			}
		}
		
		Element@ element = element_stack.pop();
		bool parent_mouse = mouse_stack.pop();
		stack_size--;
		
		while(@element != null)
		{
			if(element.visible)
			{
				element._do_layout();
				element._queue_children_for_layout(@element_stack);
				
				if(@element.parent!=null)
				{
					element.x1 = element.parent.x1 + element.x;
					element.y1 = element.parent.y1 + element.y;
				}
				else
				{
					element.x1 = element.x;
					element.y1 = element.y;
				}
				
				element.x2 = element.x1 + element.width;
				element.y2 = element.y1 + element.height;
				
				if(check_mouse_over)
				{
					if(!element.mouse_enabled)
						parent_mouse = false;
					
					if(parent_mouse)
					{
						if(element.overlaps_point(mouse.x, mouse.y))
						{
							@mouse_over = element;
						}
					}
					
					if(element.disabled || !element.children_mouse_enabled)
						parent_mouse = false;
					
					int new_stack_size = element_stack.size;
					
					while(stack_size < new_stack_size)
					{
						mouse_stack.push(parent_mouse);
						stack_size++;
					}
				}
			}
			
			@element = element_stack.pop();
			parent_mouse = mouse_stack.pop();
			stack_size--;
		}
		
		if(@mouse_over == @base)
		{
			@mouse_over = null;
		}
		
		return mouse_over;
	}
	
	private void process_mouse_events(Element@ mouse_over_root)
	{
		is_mouse_over = @_mouse_over_element != null;
		
		// /////////////////////////////////////////////////
		// Build the list/stack of the elements the mouse is over, from the innermost child to the outermost parent
		// 
		
		elements_mouse_enter.resize(0);
		
		if(is_mouse_over)
		{
			Element@ mouse_over_traversal = _mouse_over_element;
			
			do
			{
				if(!mouse_over_traversal.disabled)
				{
					elements_mouse_enter.insertLast(mouse_over_traversal);
				}
				
				@mouse_over_traversal = mouse_over_traversal.parent;
			}
			while(@mouse_over_traversal != @mouse_over_root);
		}
		
		// /////////////////////////////////////////////////
		// Mouse exit
		// 
		
		_event_info.reset(EventType::MOUSE_EXIT, MouseButton::None, mouse.x, mouse.y);
		
		const int num_elements_mouse_over = int(elements_mouse_over.size());
		const int num_elements_mouse_enter = int(elements_mouse_enter.size());
		
		for(int i = 0, j = num_elements_mouse_enter - 1; i < num_elements_mouse_over; i++, j--)
		{
			// Compare the hierarchy of the element hierarchy the mouse was over on the last frame
			// with the hierarchy this frame
			
			Element@ prev_over_element = @elements_mouse_over[i];
			Element@ new_over_element  = j >= 0 ? @elements_mouse_enter[j] : null;
			
			if(@prev_over_element == @new_over_element)
				continue;
			
			// When they no longer match, trigger mouse exit events on the entire sub tree
			
			for(int k = num_elements_mouse_over - 1; k >= i; k--)
			{
				Element@ element = @elements_mouse_over[k];
				element.hovered = false;
				
				@_event_info.target = element;
				element._mouse_exit();
				element.mouse_exit.dispatch(_event_info);
			}
			
			break;
		}
		
		// /////////////////////////////////////////////////
		// Mouse enter
		// 
		
		if(is_mouse_over)
		{
			_event_info.reset(EventType::MOUSE_ENTER, MouseButton::None, mouse.x, mouse.y);
			
			for(int i = int(elements_mouse_enter.size()) - 1; i >= 0; i--)
			{
				Element@ element = @elements_mouse_enter[i];
				
				if(element.hovered)
					continue;
				
				element.hovered = true;
				elements_mouse_over.insertLast(element);
				
				@_event_info.target = element;
				element._mouse_enter();
				element.mouse_enter.dispatch(_event_info);
			}
		}
		
		// /////////////////////////////////////////////////
		// Mouse press
		// 
		
		if(is_mouse_over && mouse.left_press || mouse.middle_press || mouse.right_press)
		{
			_event_info.reset(EventType::MOUSE_PRESS, MouseButton::None, mouse.x, mouse.y);
			
			for(int i = 0; i < num_elements_mouse_enter; i++)
			{
				Element@ element = @_event_info.target = @elements_mouse_enter[i];
				
				if(mouse.left_press)
				{
					elements_left_pressed[element._id] = true;
					element._mouse_press(_event_info.button = MouseButton::Left);
					element.mouse_press.dispatch(_event_info);
				}
				
				if(mouse.right_press)
				{
					elements_right_pressed[element._id] = true;
					element._mouse_press(_event_info.button = MouseButton::Right);
					element.mouse_press.dispatch(_event_info);
				}
				
				if(mouse.middle_press)
				{
					elements_middle_pressed[element._id] = true;
					element._mouse_press(_event_info.button = MouseButton::Middle);
					element.mouse_press.dispatch(_event_info);
				}
				
				if(mouse.primary_press)
				{
					element.pressed = true;
					elements_pressed_list.insertLast(element);
					
					// Tooltip
					if(@element.tooltip != null && element.tooltip.trigger_type == TooltipTriggerType::MouseDown)
					{
						show_tooltip(_mouse_over_element);
					}
				}
			}
		}
		
		// /////////////////////////////////////////////////
		// Mouse move
		// 
		
		if(is_mouse_over && mouse.moved)
		{
			_event_info.reset(EventType::MOUSE_MOVE, MouseButton::None, mouse.x, mouse.y);
			
			for(int i = 0; i < num_elements_mouse_enter; i++)
			{
				Element@ element = @_event_info.target = @elements_mouse_enter[i];
				element._mouse_move();
				element.mouse_move.dispatch(_event_info);
			}
		}
		
		// /////////////////////////////////////////////////
		// Mouse release and click
		// 
		
		if(is_mouse_over && mouse.left_release || mouse.middle_release || mouse.right_release)
		{
			// Release
			
			_event_info.reset(EventType::MOUSE_RELEASE, MouseButton::None, mouse.x, mouse.y);
			
			for(int i = 0; i < num_elements_mouse_enter; i++)
			{
				Element@ element = @_event_info.target = @elements_mouse_enter[i];
				
				if(mouse.left_release)
				{
					element._mouse_release(_event_info.button = MouseButton::Left);
					element.mouse_release.dispatch(_event_info);
				}
				
				if(mouse.right_release)
				{
					element._mouse_release(_event_info.button = MouseButton::Right);
					element.mouse_release.dispatch(_event_info);
				}
				
				if(mouse.middle_release)
				{
					element._mouse_release(_event_info.button = MouseButton::Middle);
					element.mouse_release.dispatch(_event_info);
				}
			}
			
			// Click
			
			_event_info.reset(EventType::MOUSE_CLICK, MouseButton::None, mouse.x, mouse.y);
			
			for(int i = 0; i < num_elements_mouse_enter; i++)
			{
				Element@ element = @_event_info.target = @elements_mouse_enter[i];
				bool primary_clicked = false;
				
				if(mouse.left_release && elements_left_pressed.exists(element._id))
				{
					_event_info.button = MouseButton::Left;
					primary_clicked = primary_clicked || primary_button == _event_info.button;
					
					if(primary_clicked)
					{
						element._mouse_click();
						element.mouse_click.dispatch(_event_info);
					}
					
					element._mouse_button_click(MouseButton::Left);
					element.mouse_button_click.dispatch(_event_info);
				}
				
				if(mouse.right_release && elements_right_pressed.exists(element._id))
				{
					_event_info.button = MouseButton::Right;
					primary_clicked = primary_clicked || primary_button == _event_info.button;
					
					if(primary_clicked)
					{
						element._mouse_click();
						element.mouse_click.dispatch(_event_info);
					}
					
					element._mouse_button_click( MouseButton::Right);
					element.mouse_button_click.dispatch(_event_info);
				}
				
				if(mouse.middle_release && elements_middle_pressed.exists(element._id))
				{
					_event_info.button = MouseButton::Middle;
					primary_clicked = primary_clicked || primary_button == _event_info.button;
					
					if(primary_clicked)
					{
						element._mouse_click();
						element.mouse_click.dispatch(_event_info);
					}
					
					element._mouse_button_click(MouseButton::Middle);
					element.mouse_button_click.dispatch(_event_info);
				}
				
				// Tooltip
				if(primary_clicked)
				{
					if(@element.tooltip != null && element.tooltip.trigger_type == TooltipTriggerType::MouseClick)
					{
						show_tooltip(_mouse_over_element);
					}
				}
			}
		}
		
		// /////////////////////////////////////////////////
		// Finalise
		// 
		
		// Set mouse_over erray
		
		elements_mouse_over.resize(elements_mouse_enter.size());
		
		for(int i = int(elements_mouse_enter.size()) - 1, j = 0; i >= 0; i--, j++)
		{
			@elements_mouse_over[j] = @elements_mouse_enter[i];
		}
		
		// Clear pressed elements
		
		if(mouse.left_release)
		{
			elements_left_pressed.deleteAll();
			
			if(primary_button == MouseButton::Left)
				clear_press();
		}
		
		if(mouse.right_release)
		{
			elements_right_pressed.deleteAll();
			
			if(primary_button == MouseButton::Right)
				clear_press();
		}
		
		if(mouse.middle_release)
		{
			elements_middle_pressed.deleteAll();
			
			if(primary_button == MouseButton::Middle)
				clear_press();
		}
		
		// Hover tooltip
		
		if(
			@_mouse_over_element != null && !_mouse_over_element.disabled &&
			@_mouse_over_element.tooltip != null &&
			_mouse_over_element.tooltip.trigger_type == TooltipTriggerType::MouseOver)
		{
			show_tooltip(_mouse_over_element);
		}
	}
	
	private void clear_press()
	{
		for(int i = int(elements_pressed_list.length()) - 1; i >= 0; i--)
		{
			elements_pressed_list[i].pressed = false;
		}
		
		elements_pressed_list.resize(0);
	}
	
	private void show_tooltip(const string id, TooltipOptions@ options, Element@ element, bool wait_for_mouse)
	{
		if(!options.enabled)
			return;
		
		if(!tooltips.exists(id))
		{
			Tooltip@ tooltip = Tooltip(this, options, element, wait_for_mouse);
			tooltip.hide.on(on_tooltip_hide_delegate);
			overlays.add_child(tooltip);
			@tooltips[id] = tooltip;
		}
		else
		{
			overlays.move_to_front(cast<Tooltip@>(tooltips[id]));
		}
	}
	
	private void hide_tooltip(const string id)
	{
		if(!tooltips.exists(id))
			return;
		
		Tooltip@ tooltip = cast<Tooltip@>(tooltips[id]);
		tooltip.force_hide();
	}
	
	private void update_tooltip(const string id, TooltipOptions@ options)
	{
		if(!tooltips.exists(id))
			return;
		
		Tooltip@ tooltip = cast<Tooltip@>(tooltips[id]);
		@tooltip.content = @options.get_contenet_element();
		tooltip.fit_to_contents();
	}
	
	private uint get_element_id_colour(Element@ element, const uint alpha=0xff)
	{
		const float hash = float(string::hash(element._id));
		return hsl_to_rgb(
			sin(hash) * 0.5 + 0.5,
			map(sin(hash) * 0.5 + 0.5, 0, 1, 0.8, 0.9),
			map(sin(hash) * 0.5 + 0.5, 0, 1, 0.65, 0.75)
		) | (alpha << 24);
	}
	
	// Events
	// ---------------------------------------------------------
	
	private void on_tooltip_hide(EventInfo@ event)
	{
		Tooltip@ tooltip = cast<Tooltip@>(event.target);
		
		if(@tooltip == null)
			return;
		
		const string id = @tooltip.target != null ? tooltip.target._id : tooltip.options._id;
		
		if(!tooltips.exists(id))
			return;
		
		tooltip.hide.off(on_tooltip_hide_delegate);
		tooltips.delete(id);
		overlays.remove_child(tooltip);
	}
	
}