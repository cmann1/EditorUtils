#include '../enums/EmitterId.cpp';

entity@ create_emitter(const int id, const float x, const float y, const int width, const int height, const int layer, const int sub_layer, const int rotation=0)
{
	entity@ emitter = create_entity('entity_emitter');
	varstruct@ vars = emitter.vars();
	emitter.layer(layer);
	vars.get_var('emitter_id').set_int32(id);
	vars.get_var('width').set_int32(width);
	vars.get_var('height').set_int32(height);
	vars.get_var('draw_depth_sub').set_int32(sub_layer);
	vars.get_var('r_area').set_bool(true);
	vars.get_var('e_rotation').set_int32(rotation);
	emitter.set_xy(x, y);
	
	return emitter;
}

int get_emitter_id_for_area(int area)
{
	switch(area)
	{
		case 1: return EmitterId::TileCleansedMansion;
		case 2: return EmitterId::TileCleansedForest;
		case 3: return EmitterId::TileCleansedCity;
		case 4: return EmitterId::TileCleansedLab;
		case 5: return EmitterId::TileCleansedVirtual;
	}
	
	return EmitterId::TileCleansedMansion;
}