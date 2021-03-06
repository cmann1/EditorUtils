/* Auto generated by .scripts/gen_prop_data.py. Do not edit. */

#include 'common.cpp';

class PropIndex
{
	
	uint set;
	uint group;
	uint index;
	uint palettes;
	uint array_index;
	string name;
	
	PropIndex() { }
	
	PropIndex(uint set, uint group, uint index, uint palettes, string name, uint array_index)
	{
		this.set = set;
		this.group = group;
		this.index = index;
		this.palettes = palettes;
		this.name = name;
		this.array_index = array_index;
	}
	
	string sprite_set_name() const
	{
		return 'props' + set;
	}
	
	string sprite_name() const
	{
		return PROP_GROUP_NAMES[group] + '_' + index;
	}
	
}

/** Takes the prop index and returns an index in PROP_INDICES  */
int prop_index_to_array_index(uint prop_set, uint prop_group, uint prop_index)
{
	string key = prop_set + '.' + prop_group + '.' + prop_index;
	
	if(PROP_INDEX_MAP.exists(key))
	{
		return int(PROP_INDEX_MAP[key]);
	}
	
	return -1;
}

/** Each item corresponds to a group defined in props.PROP_GROUP_NAMES */
const array<array<PropIndex>> PROP_INDICES = {
	{ // books
		PropIndex(1, 0, 1, 1, "Stack 1", 0),
		PropIndex(1, 0, 2, 1, "Stack 2", 1),
		PropIndex(1, 0, 3, 1, "Stack 3", 2),
		PropIndex(1, 0, 4, 1, "Stack 4", 3),
		PropIndex(1, 0, 5, 1, "Green", 4),
		PropIndex(1, 0, 6, 1, "Purple", 5),
		PropIndex(1, 0, 7, 1, "Brown Front", 6),
		PropIndex(1, 0, 8, 1, "Brown", 7),
		PropIndex(1, 0, 9, 1, "Blue", 8),
		PropIndex(1, 0, 10, 1, "Red", 9),
	},
	{ // buildingblocks
		PropIndex(1, 1, 1, 3, "Pillar", 0),
		PropIndex(1, 1, 2, 3, "Pillar Broken", 1),
		PropIndex(1, 1, 3, 3, "Piller Chunk", 2),
		PropIndex(1, 1, 4, 3, "Deco Half", 3),
		PropIndex(1, 1, 5, 3, "Deco Quarter", 4),
		PropIndex(1, 1, 6, 3, "Drain", 5),
		PropIndex(1, 1, 7, 3, "Block Large", 6),
		PropIndex(1, 1, 8, 3, "Block Small", 7),
		PropIndex(1, 1, 9, 2, "Plate", 8),
		PropIndex(1, 1, 10, 3, "Wooden Pole", 9),
	},
	{ // chains
		PropIndex(1, 2, 1, 2, "Loop Small", 0),
		PropIndex(1, 2, 2, 2, "Straight", 1),
		PropIndex(1, 2, 3, 2, "Loop Large", 2),
		PropIndex(1, 2, 4, 2, "Shackle", 3),
		PropIndex(1, 2, 5, 2, "Ball", 4),
	},
	{ // decoration
		PropIndex(1, 3, 1, 1, "Painting Old Man", 0),
		PropIndex(1, 3, 2, 1, "Painting Woman", 1),
		PropIndex(1, 3, 3, 1, "Painting Mountains", 2),
		PropIndex(1, 3, 4, 1, "Painting Still Life", 3),
		PropIndex(1, 3, 5, 1, "Painting Ocean", 4),
		PropIndex(1, 3, 6, 1, "Bust", 5),
		PropIndex(1, 3, 7, 1, "Gold Man", 6),
		PropIndex(1, 3, 8, 1, "Swords", 7),
		PropIndex(1, 3, 9, 1, "Armour", 8),
		PropIndex(2, 3, 1, 1, "Drape", 9),
	},
	{ // facade
		PropIndex(1, 4, 1, 3, "Drain", 0),
		PropIndex(1, 4, 2, 1, "Plate Small", 1),
		PropIndex(1, 4, 3, 1, "Plate Large", 2),
		PropIndex(3, 4, 1, 4, "Window Brick", 3),
		PropIndex(3, 4, 2, 2, "Window Concrete", 4),
		PropIndex(3, 4, 3, 1, "Vent", 5),
		PropIndex(3, 4, 4, 1, "Window Small", 6),
		PropIndex(3, 4, 5, 1, "Fan", 7),
		PropIndex(3, 4, 6, 1, "Fire Escape", 8),
		PropIndex(3, 4, 7, 1, "Fire Escape Stairs", 9),
		PropIndex(3, 4, 8, 1, "Ladder", 10),
		PropIndex(3, 4, 9, 1, "Chimney", 11),
		PropIndex(3, 4, 10, 1, "Gutter Pipe", 12),
		PropIndex(3, 4, 11, 1, "Gutter Pipe Top", 13),
		PropIndex(3, 4, 12, 1, "Gutter Pipe Bottom", 14),
		PropIndex(3, 4, 13, 1, "Roof Side", 15),
		PropIndex(3, 4, 14, 1, "Roof Mid", 16),
	},
	{ // foliage
		PropIndex(1, 5, 1, 1, "Shrub Side", 0),
		PropIndex(1, 5, 2, 1, "Shrub Mid", 1),
		PropIndex(1, 5, 3, 1, "Vine Small", 2),
		PropIndex(1, 5, 4, 1, "Vine Small Small", 3),
		PropIndex(1, 5, 5, 1, "Vine Small Curl", 4),
		PropIndex(1, 5, 6, 1, "Vine Thick Light", 5),
		PropIndex(1, 5, 7, 1, "Root Light", 6),
		PropIndex(1, 5, 8, 1, "Root Light Curl", 7),
		PropIndex(1, 5, 9, 1, "Flower Purple", 8),
		PropIndex(1, 5, 10, 1, "Flower Blue", 9),
		PropIndex(1, 5, 11, 1, "Flower Red", 10),
		PropIndex(1, 5, 12, 1, "Grass Medium", 11),
		PropIndex(1, 5, 13, 1, "Grass Big", 12),
		PropIndex(1, 5, 14, 1, "Grass Small", 13),
		PropIndex(1, 5, 15, 1, "Moss 1", 14),
		PropIndex(1, 5, 16, 1, "Moss 2", 15),
		PropIndex(1, 5, 17, 1, "Moss 3", 16),
		PropIndex(1, 5, 18, 1, "Moss Small", 17),
		PropIndex(1, 5, 19, 1, "Trunk 1", 18),
		PropIndex(1, 5, 20, 1, "Trunk 2", 19),
		PropIndex(2, 5, 1, 3, "Shrub Small", 20),
		PropIndex(2, 5, 2, 3, "Shrub Medium", 21),
		PropIndex(2, 5, 3, 3, "Shrub Large", 22),
		PropIndex(2, 5, 4, 3, "Grass Thick", 23),
		PropIndex(2, 5, 5, 3, "Flower", 24),
		PropIndex(2, 5, 6, 3, "Trunk Side", 25),
		PropIndex(2, 5, 7, 3, "Trunk Ground", 26),
		PropIndex(2, 5, 8, 3, "Root Large", 27),
		PropIndex(2, 5, 9, 3, "Root Small", 28),
		PropIndex(2, 5, 10, 3, "Vine Big", 29),
		PropIndex(2, 5, 11, 3, "Vine Big Small", 30),
		PropIndex(2, 5, 12, 3, "Vine Big Curl", 31),
		PropIndex(2, 5, 13, 3, "Vine Thick Dark", 32),
		PropIndex(2, 5, 14, 3, "Root Dark", 33),
		PropIndex(2, 5, 15, 3, "Root Dark Curl", 34),
		PropIndex(2, 5, 16, 1, "Stain Small", 35),
		PropIndex(2, 5, 17, 1, "Stain Medium", 36),
		PropIndex(2, 5, 18, 1, "Stain Large", 37),
		PropIndex(2, 5, 19, 1, "Mushroom Red Big", 38),
		PropIndex(2, 5, 20, 1, "Mushroom Red Group", 39),
		PropIndex(2, 5, 21, 1, "Mushroom White Group", 40),
		PropIndex(2, 5, 22, 1, "Mushroom White Big", 41),
		PropIndex(2, 5, 23, 1, "Mushroom Blue Big", 42),
		PropIndex(2, 5, 24, 1, "Mushroom Blue Group", 43),
		PropIndex(2, 5, 25, 1, "Apple", 44),
		PropIndex(2, 5, 26, 1, "Acorn", 45),
	},
	{ // furniture
		PropIndex(1, 6, 1, 1, "Bench", 0),
		PropIndex(1, 6, 2, 1, "Coffee Table", 1),
		PropIndex(1, 6, 3, 1, "Desk", 2),
		PropIndex(1, 6, 4, 1, "Chair", 3),
		PropIndex(1, 6, 5, 1, "Couch", 4),
		PropIndex(1, 6, 6, 1, "Dresser", 5),
		PropIndex(1, 6, 7, 1, "Wardrobe", 6),
		PropIndex(1, 6, 8, 1, "Door", 7),
		PropIndex(1, 6, 9, 1, "Freeman", 8),
	},
	{ // gazebo
		PropIndex(1, 7, 1, 1, "Roof Side", 0),
		PropIndex(1, 7, 2, 1, "Roof Mid", 1),
		PropIndex(1, 7, 3, 1, "Fountain", 2),
		PropIndex(1, 7, 4, 1, "Dome", 3),
		PropIndex(1, 7, 5, 1, "Piller", 4),
		PropIndex(1, 7, 6, 1, "Pillar Broken", 5),
		PropIndex(1, 7, 7, 1, "Piller Chunk", 6),
	},
	{ // lighting
		PropIndex(1, 8, 1, 1, "Candle", 0),
		PropIndex(1, 8, 2, 1, "Lamp Small Front", 1),
		PropIndex(1, 8, 3, 1, "Lamp Small Side", 2),
		PropIndex(1, 8, 4, 1, "Lamp Big Front", 3),
		PropIndex(1, 8, 5, 1, "Lamp Big Side", 4),
		PropIndex(1, 8, 6, 1, "Lamp", 5),
		PropIndex(1, 8, 7, 1, "Torch Front", 6),
		PropIndex(1, 8, 8, 1, "Torch Side", 7),
		PropIndex(1, 8, 9, 3, "Fireplace", 8),
		PropIndex(1, 8, 10, 1, "Console", 9),
		PropIndex(1, 8, 11, 1, "Computer", 10),
		PropIndex(2, 8, 1, 1, "Forest Lamp Standing", 11),
		PropIndex(2, 8, 2, 1, "Forest Lamp", 12),
		PropIndex(3, 8, 1, 1, "City Lamp Standing", 13),
		PropIndex(3, 8, 2, 1, "City Lamp", 14),
	},
	{},
	{ // statues
		PropIndex(1, 10, 1, 3, "Gargoyle Front", 0),
		PropIndex(1, 10, 2, 3, "Gargoyle Side", 1),
	},
	{ // storage
		PropIndex(1, 11, 1, 1, "Barrel", 0),
		PropIndex(1, 11, 2, 1, "Barrel Side", 1),
		PropIndex(1, 11, 3, 1, "Crate Big", 2),
		PropIndex(1, 11, 4, 1, "Crate Small", 3),
	},
	{ // study
		PropIndex(1, 12, 1, 1, "Globe", 0),
		PropIndex(1, 12, 2, 1, "Telescope", 1),
		PropIndex(1, 12, 3, 1, "Gramophone", 2),
		PropIndex(1, 12, 4, 1, "Piano", 3),
	},
	{ // fencing
		PropIndex(1, 13, 1, 3, "Balustrade Mid", 0),
		PropIndex(1, 13, 2, 3, "Balustrade Side", 1),
		PropIndex(1, 13, 3, 2, "Fence Mid", 2),
		PropIndex(1, 13, 4, 2, "Fence Side", 3),
		PropIndex(1, 13, 5, 1, "Lattice Large", 4),
		PropIndex(1, 13, 6, 1, "Lattice Small", 5),
	},
	{},
	{},
	{},
	{},
	{ // backleaves
		PropIndex(2, 18, 1, 3, "Leaves 1", 0),
		PropIndex(2, 18, 2, 3, "Leaves 2", 1),
		PropIndex(2, 18, 3, 3, "Leaves 3", 2),
		PropIndex(2, 18, 4, 3, "Leaves 4", 3),
		PropIndex(2, 18, 5, 3, "Leaves 5", 4),
		PropIndex(2, 18, 6, 3, "Leaves 6", 5),
		PropIndex(2, 18, 7, 3, "Leaves 7", 6),
		PropIndex(2, 18, 8, 3, "Leaves 8", 7),
	},
	{ // leaves
		PropIndex(2, 19, 1, 3, "Leaves 1", 0),
		PropIndex(2, 19, 2, 3, "Leaves 2", 1),
		PropIndex(2, 19, 3, 3, "Leaves 3", 2),
		PropIndex(2, 19, 4, 3, "Leaves 4", 3),
		PropIndex(2, 19, 5, 3, "Leaves 5", 4),
		PropIndex(2, 19, 6, 3, "Leaves 6", 5),
		PropIndex(2, 19, 7, 3, "Leaves 7", 6),
		PropIndex(2, 19, 8, 3, "Leaves 8", 7),
	},
	{ // trunks
		PropIndex(2, 20, 1, 3, "Trunk 1", 0),
		PropIndex(2, 20, 2, 3, "Trunk 2", 1),
		PropIndex(2, 20, 3, 3, "Trunk Fat", 2),
		PropIndex(2, 20, 4, 3, "Trunk 3", 3),
		PropIndex(2, 20, 5, 3, "Trunk 4", 4),
		PropIndex(2, 20, 6, 3, "Trunk 5", 5),
		PropIndex(2, 20, 7, 3, "Trunk 6", 6),
		PropIndex(2, 20, 8, 3, "Trunk Thin", 7),
	},
	{ // boulders
		PropIndex(2, 21, 1, 2, "Boulder Round Small", 0),
		PropIndex(2, 21, 2, 2, "Boulder Round Medium", 1),
		PropIndex(2, 21, 3, 2, "Boulder Round Large", 2),
		PropIndex(2, 21, 4, 2, "Boulder Tri Small", 3),
		PropIndex(2, 21, 5, 2, "Boulder Tri Medium", 4),
		PropIndex(2, 21, 6, 2, "Boulder Tri Large", 5),
		PropIndex(2, 21, 7, 1, "Logo Large", 6),
		PropIndex(2, 21, 8, 1, "Logo Medium", 7),
		PropIndex(2, 21, 9, 1, "Logo Small", 8),
		PropIndex(2, 21, 10, 1, "Logo Xlarge", 9),
		PropIndex(2, 21, 11, 1, "Face 1", 10),
		PropIndex(2, 21, 12, 1, "Face 2", 11),
		PropIndex(2, 21, 13, 1, "Vacuum", 12),
		PropIndex(2, 21, 14, 1, "Broom", 13),
		PropIndex(2, 21, 15, 1, "Broom Push", 14),
		PropIndex(2, 21, 16, 1, "Dusters", 15),
	},
	{ // backdrops
		PropIndex(1, 22, 1, 1, "Cloud Black 1", 0),
		PropIndex(1, 22, 2, 1, "Cloud Black 2", 1),
		PropIndex(1, 22, 3, 1, "Cloud Black 3", 2),
		PropIndex(1, 22, 4, 1, "Moon", 3),
		PropIndex(1, 22, 5, 1, "Black Water", 4),
		PropIndex(1, 22, 6, 1, "Black Rect", 5),
		PropIndex(2, 22, 1, 1, "Cloud White 1", 6),
		PropIndex(2, 22, 2, 1, "Cloud White 2", 7),
		PropIndex(2, 22, 3, 1, "Cloud White 3", 8),
		PropIndex(2, 22, 4, 1, "Sun", 9),
		PropIndex(2, 22, 5, 1, "Mountain", 10),
		PropIndex(2, 22, 6, 1, "Tree Tops", 11),
		PropIndex(2, 22, 7, 1, "Green Rect", 12),
		PropIndex(3, 22, 1, 1, "City Off", 13),
		PropIndex(3, 22, 2, 1, "City On", 14),
		PropIndex(3, 22, 3, 1, "Ellipse", 15),
		PropIndex(3, 22, 4, 1, "Light Beam", 16),
		PropIndex(3, 22, 5, 4, "Building Small Off", 17),
		PropIndex(3, 22, 6, 4, "Building Small On", 18),
		PropIndex(3, 22, 7, 4, "Building Off", 19),
		PropIndex(3, 22, 8, 4, "Building On", 20),
		PropIndex(4, 22, 1, 2, "Satellite Dish", 21),
		PropIndex(4, 22, 2, 2, "Structures Wide", 22),
		PropIndex(4, 22, 3, 2, "Structures Fat", 23),
	},
	{ // temple
		PropIndex(2, 23, 1, 2, "Pillar", 0),
		PropIndex(2, 23, 2, 2, "Pillar Broken", 1),
		PropIndex(2, 23, 3, 2, "Piller Chunk", 2),
		PropIndex(2, 23, 4, 2, "Head Small", 3),
		PropIndex(2, 23, 5, 2, "Head Big", 4),
	},
	{ // npc
		PropIndex(5, 24, 1, 1, "Virtual", 0),
		PropIndex(6, 24, 1, 3, "Leaf", 1),
		PropIndex(6, 24, 2, 1, "Maid", 2),
		PropIndex(6, 24, 3, 1, "Butler", 3),
		PropIndex(6, 24, 4, 1, "Worker", 4),
		PropIndex(6, 24, 5, 1, "Scientist", 5),
	},
	{ // symbol
		PropIndex(5, 25, 1, 1, "Arrow", 0),
		PropIndex(5, 25, 2, 1, "Warning", 1),
	},
	{ // cars
		PropIndex(3, 26, 1, 2, "Cleaner", 0),
		PropIndex(3, 26, 2, 3, "Truck", 1),
		PropIndex(3, 26, 3, 3, "Car", 2),
	},
	{ // sidewalk
		PropIndex(3, 27, 1, 1, "Hydrant", 0),
		PropIndex(3, 27, 2, 1, "Fence Side", 1),
		PropIndex(3, 27, 3, 1, "Fence Mid", 2),
		PropIndex(3, 27, 4, 1, "Telephone Pole", 3),
		PropIndex(3, 27, 5, 1, "Wire Loop", 4),
		PropIndex(3, 27, 6, 1, "Wire Bundle", 5),
		PropIndex(3, 27, 7, 1, "Wire", 6),
		PropIndex(3, 27, 8, 1, "Wire Hanging", 7),
		PropIndex(3, 27, 9, 1, "Gear Side", 8),
		PropIndex(3, 27, 10, 1, "Gear Big", 9),
		PropIndex(3, 27, 11, 1, "Gear Small", 10),
	},
	{ // machinery
		PropIndex(4, 28, 1, 2, "Panel Big", 0),
		PropIndex(4, 28, 2, 2, "Panel Small", 1),
		PropIndex(4, 28, 3, 2, "Panel Medium", 2),
		PropIndex(4, 28, 4, 2, "Tall", 3),
		PropIndex(4, 28, 5, 2, "Fat", 4),
		PropIndex(4, 28, 6, 2, "Gasket Big", 5),
		PropIndex(4, 28, 7, 1, "Pipe Big", 6),
		PropIndex(4, 28, 8, 1, "Pipe Big Corner", 7),
		PropIndex(4, 28, 9, 2, "Gasket Small", 8),
		PropIndex(4, 28, 10, 1, "Pipe Small", 9),
		PropIndex(4, 28, 11, 1, "Pipe Small Corner", 10),
		PropIndex(4, 28, 12, 2, "Tube", 11),
		PropIndex(4, 28, 13, 1, "Wire", 12),
		PropIndex(4, 28, 14, 1, "Wire Straight", 13),
		PropIndex(4, 28, 15, 1, "Wire Corner", 14),
		PropIndex(4, 28, 16, 1, "Wire Coloured", 15),
		PropIndex(4, 28, 17, 1, "Wire Coloured Corner", 16),
		PropIndex(4, 28, 18, 2, "Light Big", 17),
		PropIndex(4, 28, 19, 2, "Light Small", 18),
	},
};

/** Takes the prop index and returns an index in PROP_INDICES  */
const dictionary PROP_INDEX_MAP = {
	{'1.0.1', 0},
	{'1.0.2', 1},
	{'1.0.3', 2},
	{'1.0.4', 3},
	{'1.0.5', 4},
	{'1.0.6', 5},
	{'1.0.7', 6},
	{'1.0.8', 7},
	{'1.0.9', 8},
	{'1.0.10', 9},
	{'1.1.1', 0},
	{'1.1.2', 1},
	{'1.1.3', 2},
	{'1.1.4', 3},
	{'1.1.5', 4},
	{'1.1.6', 5},
	{'1.1.7', 6},
	{'1.1.8', 7},
	{'1.1.9', 8},
	{'1.1.10', 9},
	{'1.2.1', 0},
	{'1.2.2', 1},
	{'1.2.3', 2},
	{'1.2.4', 3},
	{'1.2.5', 4},
	{'1.3.1', 0},
	{'1.3.2', 1},
	{'1.3.3', 2},
	{'1.3.4', 3},
	{'1.3.5', 4},
	{'1.3.6', 5},
	{'1.3.7', 6},
	{'1.3.8', 7},
	{'1.3.9', 8},
	{'1.4.1', 0},
	{'1.4.2', 1},
	{'1.4.3', 2},
	{'1.13.1', 0},
	{'1.13.2', 1},
	{'1.13.3', 2},
	{'1.13.4', 3},
	{'1.13.5', 4},
	{'1.13.6', 5},
	{'1.5.1', 0},
	{'1.5.2', 1},
	{'1.5.3', 2},
	{'1.5.4', 3},
	{'1.5.5', 4},
	{'1.5.6', 5},
	{'1.5.7', 6},
	{'1.5.8', 7},
	{'1.5.9', 8},
	{'1.5.10', 9},
	{'1.5.11', 10},
	{'1.5.12', 11},
	{'1.5.13', 12},
	{'1.5.14', 13},
	{'1.5.15', 14},
	{'1.5.16', 15},
	{'1.5.17', 16},
	{'1.5.18', 17},
	{'1.5.19', 18},
	{'1.5.20', 19},
	{'1.6.1', 0},
	{'1.6.2', 1},
	{'1.6.3', 2},
	{'1.6.4', 3},
	{'1.6.5', 4},
	{'1.6.6', 5},
	{'1.6.7', 6},
	{'1.6.8', 7},
	{'1.6.9', 8},
	{'1.7.1', 0},
	{'1.7.2', 1},
	{'1.7.3', 2},
	{'1.7.4', 3},
	{'1.7.5', 4},
	{'1.7.6', 5},
	{'1.7.7', 6},
	{'1.8.1', 0},
	{'1.8.2', 1},
	{'1.8.3', 2},
	{'1.8.4', 3},
	{'1.8.5', 4},
	{'1.8.6', 5},
	{'1.8.7', 6},
	{'1.8.8', 7},
	{'1.8.9', 8},
	{'1.8.10', 9},
	{'1.8.11', 10},
	{'1.10.1', 0},
	{'1.10.2', 1},
	{'1.11.1', 0},
	{'1.11.2', 1},
	{'1.11.3', 2},
	{'1.11.4', 3},
	{'1.12.1', 0},
	{'1.12.2', 1},
	{'1.12.3', 2},
	{'1.12.4', 3},
	{'1.22.1', 0},
	{'1.22.2', 1},
	{'1.22.3', 2},
	{'1.22.4', 3},
	{'1.22.5', 4},
	{'1.22.6', 5},
	{'2.18.1', 0},
	{'2.18.2', 1},
	{'2.18.3', 2},
	{'2.18.4', 3},
	{'2.18.5', 4},
	{'2.18.6', 5},
	{'2.18.7', 6},
	{'2.18.8', 7},
	{'2.19.1', 0},
	{'2.19.2', 1},
	{'2.19.3', 2},
	{'2.19.4', 3},
	{'2.19.5', 4},
	{'2.19.6', 5},
	{'2.19.7', 6},
	{'2.19.8', 7},
	{'2.20.1', 0},
	{'2.20.2', 1},
	{'2.20.3', 2},
	{'2.20.4', 3},
	{'2.20.5', 4},
	{'2.20.6', 5},
	{'2.20.7', 6},
	{'2.20.8', 7},
	{'2.21.1', 0},
	{'2.21.2', 1},
	{'2.21.3', 2},
	{'2.21.4', 3},
	{'2.21.5', 4},
	{'2.21.6', 5},
	{'2.21.7', 6},
	{'2.21.8', 7},
	{'2.21.9', 8},
	{'2.21.10', 9},
	{'2.21.11', 10},
	{'2.21.12', 11},
	{'2.21.13', 12},
	{'2.21.14', 13},
	{'2.21.15', 14},
	{'2.21.16', 15},
	{'2.8.1', 11},
	{'2.8.2', 12},
	{'2.3.1', 9},
	{'2.5.1', 20},
	{'2.5.2', 21},
	{'2.5.3', 22},
	{'2.5.4', 23},
	{'2.5.5', 24},
	{'2.5.6', 25},
	{'2.5.7', 26},
	{'2.5.8', 27},
	{'2.5.9', 28},
	{'2.5.10', 29},
	{'2.5.11', 30},
	{'2.5.12', 31},
	{'2.5.13', 32},
	{'2.5.14', 33},
	{'2.5.15', 34},
	{'2.5.16', 35},
	{'2.5.17', 36},
	{'2.5.18', 37},
	{'2.5.19', 38},
	{'2.5.20', 39},
	{'2.5.21', 40},
	{'2.5.22', 41},
	{'2.5.23', 42},
	{'2.5.24', 43},
	{'2.5.25', 44},
	{'2.5.26', 45},
	{'2.23.1', 0},
	{'2.23.2', 1},
	{'2.23.3', 2},
	{'2.23.4', 3},
	{'2.23.5', 4},
	{'2.22.1', 6},
	{'2.22.2', 7},
	{'2.22.3', 8},
	{'2.22.4', 9},
	{'2.22.5', 10},
	{'2.22.6', 11},
	{'2.22.7', 12},
	{'3.22.1', 13},
	{'3.22.2', 14},
	{'3.22.3', 15},
	{'3.22.4', 16},
	{'3.22.5', 17},
	{'3.22.6', 18},
	{'3.22.7', 19},
	{'3.22.8', 20},
	{'3.4.1', 3},
	{'3.4.2', 4},
	{'3.4.3', 5},
	{'3.4.4', 6},
	{'3.4.5', 7},
	{'3.4.6', 8},
	{'3.4.7', 9},
	{'3.4.8', 10},
	{'3.4.9', 11},
	{'3.4.10', 12},
	{'3.4.11', 13},
	{'3.4.12', 14},
	{'3.4.13', 15},
	{'3.4.14', 16},
	{'3.26.1', 0},
	{'3.26.2', 1},
	{'3.26.3', 2},
	{'3.27.1', 0},
	{'3.27.2', 1},
	{'3.27.3', 2},
	{'3.27.4', 3},
	{'3.27.5', 4},
	{'3.27.6', 5},
	{'3.27.7', 6},
	{'3.27.8', 7},
	{'3.27.9', 8},
	{'3.27.10', 9},
	{'3.27.11', 10},
	{'3.8.1', 13},
	{'3.8.2', 14},
	{'4.28.1', 0},
	{'4.28.2', 1},
	{'4.28.3', 2},
	{'4.28.4', 3},
	{'4.28.5', 4},
	{'4.28.6', 5},
	{'4.28.7', 6},
	{'4.28.8', 7},
	{'4.28.9', 8},
	{'4.28.10', 9},
	{'4.28.11', 10},
	{'4.28.12', 11},
	{'4.28.13', 12},
	{'4.28.14', 13},
	{'4.28.15', 14},
	{'4.28.16', 15},
	{'4.28.17', 16},
	{'4.28.18', 17},
	{'4.28.19', 18},
	{'4.22.1', 21},
	{'4.22.2', 22},
	{'4.22.3', 23},
	{'5.24.1', 0},
	{'5.25.1', 0},
	{'5.25.2', 1},
	{'6.24.1', 1},
	{'6.24.2', 2},
	{'6.24.3', 3},
	{'6.24.4', 4},
	{'6.24.5', 5}
};