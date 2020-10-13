import Models;

trait WorldGen {
    int[] generate(int x, int y, int z); // requires arr.size() == 16*16*16
}

class OverworldGen {}
implement OverworldGen {}

implement WorldGen on OverworldGen {
    int calc_coords(int x, int y, int z) {
        return 16*16*x + 16*y + z;
    }
    int[] generate(int x, int y, int z) {
        // x, y, z in chunk coords
        int[] ret = [];
        ret.resize(16*16*16);
        // ret[16*16*x+16*y+z] = 
        
        int stone_height = -8;
        int dirt_height = 0;

        int dirt = models.dirt_model;
        int stone = models.stone_model;
        
        for(int dx = 0; dx < 16; dx++){
            for(int dy = 0; dy < 16; dy++){
                for(int dz = 0; dz < 16; dz++){
                    int i = this.calc_coords(dx, dy, dz);
                    if(y >= 0) {
                        ret[i] = 0;
                    } else if(y == -1) {
                        if(dy >= 8) {
                            ret[i] = dirt;
                        } else {
                            ret[i] = stone;
                        }
                    } else {
                        ret[i] = stone;
                    }
                }
            }
        }

        return ret;
    }
}

OverworldGen overworld_generator = new OverworldGen();

export {overworld_generator};
