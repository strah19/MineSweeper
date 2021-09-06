#include "Core/Application.h"
#include "Animation/Spritesheet.h"
#include "Gui/Button.h"
#include <unordered_map>
#include <iomanip>

int grid_x = 0;
int grid_y = 100;

const int square_width = 32;
const int square_height = 32;

const int width = 16;
const int height = 16;

const int MINE_ID = -2;
int percentage_of_mines = 8; //in percentage

const int EMPTY_ID = -1;
const int COVERED_ID = 0;

const int ONE_ID = 1;
const int TWO_ID = 2;
const int THREE_ID = 3;
const int FOUR_ID = 4;
const int FIVE_ID = 5;
const int SIX_ID = 6;
const int SEVEN_ID = 7;
const int EIGHT_ID = 8;
const int MINE_HIT_ID = 9;

const int FLAG_ID = 10;
const int CROSSED_MINE_ID = 11;

const int HAPPY_FACE_OUT = 12;
const int HAPPY_FACE_IN = 13;
const int OOO_FACE = 14;
const int DEAD_FACE = 15;
const int COOL_FACE = 17;

const int NOT_MINE_ID = 16;

const int SEVEN_SEG_ZERO = 20;
const int SEVEN_SEG_ONE = 21;
const int SEVEN_SEG_TWO = 22;
const int SEVEN_SEG_THREE = 23;
const int SEVEN_SEG_FOUR = 24;
const int SEVEN_SEG_FIVE = 25;
const int SEVEN_SEG_SIX = 26;
const int SEVEN_SEG_SEVEN = 27;
const int SEVEN_SEG_EIGHT = 28;
const int SEVEN_SEG_NINE = 29;

std::unordered_map<int, Ember::IVec2> spritesheet_ids;
Ember::IVec2 check_neigbors[8] = {
	{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}
};

class SevenSegmentDisplay {
public:
	void Init(int s) {
		size = s;
	}
	void Draw(Ember::Texture& texture, Ember::SpriteSheet& spritesheet, int time, const Ember::IVec2& pos) {
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(size) << time;

		int offset = 0;
		for (int i = 0; i < size; i++) {
			int t = (int)ss.str()[i] - 48;
			Ember::IVec2 spritesheet_id = spritesheet_ids[t + SEVEN_SEG_ZERO];
			spritesheet.SelectSprite(spritesheet_id.x, spritesheet_id.y);
			texture.Draw(Ember::Rect({ pos.x + offset, pos.y, 32, 64 }), spritesheet.ReturnSourceRect());
			offset += 32;
		}

	}

private:
	int size = 0;
};

class MineSweeper : public Ember::Application {
public:
	void OnCreate() {
		in_grid_texture.Initialize("res/grid-sprites.png", renderer);
		in_grid_spritesheet.Initialize(in_grid_texture, 2, 8);

		faces_texture.Initialize("res/faces.png", renderer);
		faces_spritesheet.Initialize(faces_texture, 1, 5);

		number_texture.Initialize("res/nums.png", renderer);
		number_spritesheet.Initialize(number_texture, 1, 10);

		spritesheet_ids[COVERED_ID] = { 0, 0 };
		spritesheet_ids[EMPTY_ID] = { 1, 0 };
		spritesheet_ids[MINE_ID] = { 5, 0 };
		spritesheet_ids[MINE_HIT_ID] = { 6, 0 };
		spritesheet_ids[FLAG_ID] = { 2, 0 };
		spritesheet_ids[CROSSED_MINE_ID] = {7, 0};
		spritesheet_ids[HAPPY_FACE_OUT] = { 0, 0 };
		spritesheet_ids[HAPPY_FACE_IN] = { 1, 0 };
		spritesheet_ids[OOO_FACE] = { 2, 0 };
		spritesheet_ids[DEAD_FACE] = { 4, 0 };
		spritesheet_ids[NOT_MINE_ID] = { 7, 0 };
		spritesheet_ids[COOL_FACE] = { 3, 0 };

		play_button.Initialize(events, play_button_position);

		for (int i = 0; i < 8; i++) {
			spritesheet_ids[i + 1] = { i, 1 };
		}

		for (int i = SEVEN_SEG_ZERO; i <= SEVEN_SEG_NINE; i++) {
			spritesheet_ids[i] = { i - SEVEN_SEG_ZERO, 0 };
		}

		timer.Init(3);
		flags_left.Init(3);
		EmptyMines();
		FillArenaWithMines();
		FillArenaWithNumbers();
		face_spritesheet_id = spritesheet_ids[HAPPY_FACE_OUT];
	}

	void EmptyMines() {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				arena[y * width + x] = ArenaSquare(EMPTY_ID);
				arena[y * width + x].covered = true;
				arena[y * width + x].top_id = COVERED_ID;;
			}
		}
	}

	bool InBounds(const Ember::IVec2& selected) {
		return (selected.x >= 0 && selected.x < width && selected.y >= 0 && selected.y < height);
	}

	void CheckWin() {
		bool won = true;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (arena[y * width + x].covered && arena[y * width + x].id != MINE_ID) {
					won = false;
				}
			}
		}

		if (won) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					if (arena[y * width + x].id == MINE_ID) {
						arena[y * width + x].id = FLAG_ID;
						arena[y * width + x].top_id = FLAG_ID;
					}
				}
			}
			game_over = true;
			face_spritesheet_id = spritesheet_ids[COOL_FACE];
		}
	}

	void CheckFlagWin() {
		bool won = false;
		int mine_count = 0;
		int total = 0;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (arena[y * width + x].id == MINE_ID)
					total++;
				if (arena[y * width + x].top_id == FLAG_ID && arena[y * width + x].id == MINE_ID) {
					mine_count++;
				}
			}
		}
		if (total == mine_count)
			won = true;
		if (won) {
			game_over = true;
			face_spritesheet_id = spritesheet_ids[COOL_FACE];
		}
	}

	void UnCover(const Ember::IVec2& selected_square) {
		Ember::IVec2 current = selected_square;
		for (int i = 0; i < 8; i++) {
			if (InBounds(current + check_neigbors[i])) {
				if ((arena[(current.y + check_neigbors[i].y) * width + (current.x + check_neigbors[i].x)].id == EMPTY_ID && 
					arena[(current.y + check_neigbors[i].y) * width + (current.x + check_neigbors[i].x)].covered) &&
					arena[(current.y + check_neigbors[i].y) * width + (current.x + check_neigbors[i].x)].top_id != FLAG_ID) {
					
					arena[(current.y + check_neigbors[i].y) * width + (current.x + check_neigbors[i].x)].covered = false;
					for (int j = 0; j < 8; j++) {
						if (InBounds(current + check_neigbors[i] + check_neigbors[j])) {
							if (arena[(current.y + check_neigbors[i].y + check_neigbors[j].y) * width + (current.x + check_neigbors[i].x + check_neigbors[j].x)].id != EMPTY_ID &&
								arena[(current.y + check_neigbors[i].y + check_neigbors[j].y) * width + (current.x + check_neigbors[i].x + check_neigbors[j].x)].id != MINE_ID &&
								arena[(current.y + check_neigbors[i].y + check_neigbors[j].y) * width + (current.x + check_neigbors[i].x + check_neigbors[j].x)].top_id != FLAG_ID) {
								arena[(current.y + check_neigbors[i].y + check_neigbors[j].y) * width + (current.x + check_neigbors[i].x + check_neigbors[j].x)].covered = false;
							}
						}
					}
					current += check_neigbors[i];
					UnCover(current);
					current -= check_neigbors[i];
				}
			}
		}
	}

	void FillArenaWithMines() {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int add_mine = rand() % 100;
				if (add_mine <= percentage_of_mines) {
					arena[y * width + x] = ArenaSquare(MINE_ID);
					flag_c++;
				}
			}
		}
	}

	void FillArenaWithNumbers() {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (arena[y * width + x].id != MINE_ID) {
					int mine_num = 0;
					for (int i = 0; i < 8; i++) {
						if (InBounds({ x + check_neigbors[i].x, y + check_neigbors[i].y })) {
							if (arena[(y + check_neigbors[i].y) * width + (x + check_neigbors[i].x)].id == MINE_ID) {
								mine_num++;
							}

							if (mine_num > 0) {
								arena[y * width + x].id = mine_num;
							}
						}
					}
				}
			}
		}
	}

	void UnCoverMines() {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if(arena[y * width + x].id == MINE_ID)
					arena[y * width + x].covered = false;
			}
		}
	}

	void CheckFlags() {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (arena[y * width + x].top_id == FLAG_ID && arena[y * width + x].id != MINE_ID) {
					arena[y * width + x].id = NOT_MINE_ID;
					arena[y * width + x].top_id = NOT_MINE_ID;
				}
			}
		}
	}

	virtual ~MineSweeper() { }

	void Reset() {
		past_seconds += seconds_pass;
		flag_c = 0;
		seconds_pass = 0;
		EmptyMines();
		FillArenaWithMines();
		FillArenaWithNumbers();
		face_spritesheet_id = spritesheet_ids[HAPPY_FACE_OUT];
		game_over = false;
	}

	void OnUserUpdate() {
		seconds_pass = (SDL_GetTicks() / 1000) - past_seconds;

		window->Update();

		if (!game_over) {
			int square_select_x = (events->MousePosition().x - grid_x) / square_width;
			int square_select_y = (events->MousePosition().y - grid_y) / square_height;

			if (!events->Down())
				face_spritesheet_id = spritesheet_ids[HAPPY_FACE_OUT];

			if ((past_square_select_x != square_select_x || past_square_select_y != square_select_y) && arena[past_square_select_y * width + past_square_select_x].top_id != FLAG_ID) {
				arena[past_square_select_y * width + past_square_select_x].top_id = COVERED_ID;
			}
			if (square_select_x >= 0 && square_select_x < width) {
				if (square_select_y >= 0 && square_select_y < height) {
					if (arena[square_select_y * width + square_select_x].top_id == EMPTY_ID && !events->Down()) {
						arena[square_select_y * width + square_select_x].covered = false;
						CheckWin();
						if (arena[square_select_y * width + square_select_x].id == MINE_ID) {
							CheckFlags();
							UnCoverMines();
							face_spritesheet_id = spritesheet_ids[DEAD_FACE];
							arena[square_select_y * width + square_select_x].id = MINE_HIT_ID;
							game_over = true;
						}
						if (arena[square_select_y * width + square_select_x].id == EMPTY_ID) {
							Ember::IVec2 current = { square_select_x, square_select_y };
							for (int i = 0; i < 8; i++) {
								current += check_neigbors[i];
								arena[current.y * width + current.x].covered = false;
								UnCover(current);
								current = { square_select_x, square_select_y };
							}
						}
					}
					if (events->Down() && events->ButtonId() == Ember::ButtonIds::LeftMouseButton && arena[square_select_y * width + square_select_x].top_id != FLAG_ID
						&& arena[past_square_select_y * width + past_square_select_x].top_id != FLAG_ID) {

						arena[square_select_y * width + square_select_x].top_id = EMPTY_ID;
						face_spritesheet_id = spritesheet_ids[OOO_FACE]; 
					}
					else if(events->Down() && events->ButtonId() == Ember::ButtonIds::LeftMouseButton && arena[square_select_y * width + square_select_x].top_id == FLAG_ID){
						face_spritesheet_id = spritesheet_ids[HAPPY_FACE_OUT];
					}

					if (events->Clicked() && events->ButtonId() == Ember::ButtonIds::RightMouseButton && arena[square_select_y * width + square_select_x].covered) {
						if (arena[square_select_y * width + square_select_x].top_id == FLAG_ID) {
							arena[square_select_y * width + square_select_x].top_id = COVERED_ID;
							arena[square_select_y * width + square_select_x].covered = true;
							flag_c++;
						}
						else {
							if (flag_c > 0) {
								CheckFlagWin();
								arena[square_select_y * width + square_select_x].top_id = FLAG_ID;
								flag_c--;
							}
						}
					}
				}
			}

			past_square_select_x = square_select_x;
			past_square_select_y = square_select_y;
		}

		if (pressing && !events->Down()) {
			Reset();
		}
		if (play_button.Hold(Ember::ButtonIds::LeftMouseButton))
			pressing = true;
		else
			pressing = false;
		if (pressing) {
			face_spritesheet_id = spritesheet_ids[HAPPY_FACE_IN];
		}

		renderer->Clear(background_color);
		
		renderer->Rectangle(Ember::Rect({ 0, 0, properties->width, grid_y }), { 192, 192, 192, 255 });

		timer.Draw(number_texture, number_spritesheet, seconds_pass, { properties->width - (3 * 32), (grid_y / 2) - (64 / 2) });
		flags_left.Draw(number_texture, number_spritesheet, flag_c, { 0, (grid_y / 2) - (64 / 2) });

		faces_spritesheet.SelectSprite(face_spritesheet_id.x, face_spritesheet_id.y);
		faces_texture.Draw(play_button_position, faces_spritesheet.ReturnSourceRect());

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Ember::IVec2 spritesheet_id = spritesheet_ids[COVERED_ID];
				if (!arena[y * width + x].covered) {
					spritesheet_id = spritesheet_ids[arena[y * width + x].id];
					in_grid_spritesheet.SelectSprite(spritesheet_id.x, spritesheet_id.y);
				}
				if(arena[y * width + x].covered || arena[y * width + x].top_id == FLAG_ID) {
					spritesheet_id = spritesheet_ids[arena[y * width + x].top_id];
					in_grid_spritesheet.SelectSprite(spritesheet_id.x, spritesheet_id.y);
				}
				in_grid_texture.Draw(Ember::Rect({ x * square_width + grid_x, y * square_height + grid_y, square_width, square_height }), in_grid_spritesheet.ReturnSourceRect());
			}
		}

		renderer->Show();
	}

	bool Keyboard(Ember::KeyboardEvents& keyboard) {
		if (keyboard.scancode == Ember::EmberKeyCode::Escape) {
			window->Quit();
		}
		return true;
	}

	void UserDefEvent(Ember::Event& event) {
		Ember::EventDispatcher dispatch(&event);
		dispatch.Dispatch<Ember::KeyboardEvents>(EMBER_BIND_FUNC(Keyboard));
	}
private:
	struct ArenaSquare {
		ArenaSquare() : id(EMPTY_ID) { }
		ArenaSquare(int id) : id(id) { }

		bool covered = true;
		int id;
		int top_id = COVERED_ID;
	};

	Ember::Color background_color = { 0, 0, 0, 255 };

	ArenaSquare arena[width * height];


	Ember::Texture in_grid_texture;
	Ember::SpriteSheet in_grid_spritesheet;

	Ember::Texture faces_texture;
	Ember::SpriteSheet faces_spritesheet;

	Ember::Texture number_texture;
	Ember::SpriteSheet number_spritesheet;

	Ember::Button play_button;
	Ember::IVec2 button_size = { 52, 52 };
	Ember::Rect play_button_position = { ((width * square_width) / 2) - (button_size.x / 2), (grid_y / 2) - (button_size.y / 2), 
		button_size.x, button_size.y };
	bool pressing = false;
	int past_square_select_x = 0;
	int past_square_select_y = 0;
	bool game_over = false;
	Ember::IVec2 face_spritesheet_id;
	Uint32 seconds_pass = 0;
	Uint32 past_seconds = 0;
	SevenSegmentDisplay timer;
	SevenSegmentDisplay flags_left;
	int flag_c = 0;
};

int main(int argc, char** argv) {
	srand((unsigned int)time(NULL));

	MineSweeper mine_sweeper;
	mine_sweeper.Initialize("MineSweeper", false, grid_x + (width * square_width), grid_y + (height * square_height));

	mine_sweeper.Run();

	return 0;
}