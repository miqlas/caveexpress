#include "tests/TestShared.h"
#include "cavepacker/server/map/BoardState.h"

namespace cavepacker {

class BoardStateTest: public AbstractTest {
protected:
	void fillState(BoardState& s, const char* board, bool convertPlayers = true) const {
		int col = 0;
		int row = 0;
		int maxCol = 0;
		const char *d = board;
		while (*d != '\0') {
			if (*d == '\n') {
				if (*(d + 1) == '\0')
					break;
				++row;
				maxCol = std::max(maxCol, col);
				col = 0;
			} else {
				++col;
			}
			++d;
		}
		// we need the size for proper index calculations
		s.setSize(maxCol, row + 1);
		// after finding out the size - let's fill the board
		d = board;
		col = 0;
		row = 0;
		while (*d != '\0') {
			if (*d == '\n') {
				if (*(d + 1) == '\0')
					break;
				++row;
				col = 0;
			} else {
				char c = *d;
				// usually other players block the movement, but for the test we just ignore this
				if (convertPlayers && (c == Sokoban::PLAYER || c == Sokoban::PLAYERONTARGET))
					c = Sokoban::GROUND;
				ASSERT_TRUE(s.setField(col, row, c)) << "Could not set the field " << c << " at " << col << ":" << row;
				++col;
			}
			++d;
		}
		// at least 3 rows are needed
		ASSERT_GE(row, 2) << "could not fill the board state with " << board;
	}
};


TEST_F(BoardStateTest, testFillState) {
	{
		BoardState s;
		const char* mapStr =
				"#####\n"
				"#@$.#\n"
				"#####";
		fillState(s, mapStr);
		ASSERT_EQ(5, s.getWidth());
		ASSERT_EQ(3, s.getHeight());
	}
	{
		BoardState s;
		const char* mapStr =
			"#####\n"
			"#@$.#\n"
			"#   #\n"
			"#   ####\n"
			"#      #\n"
			"########";
		fillState(s, mapStr);
		ASSERT_EQ(8, s.getWidth());
		ASSERT_EQ(6, s.getHeight());
	}
}

TEST_F(BoardStateTest, testDeadlock1) {
	BoardState s;

	const char* mapStr =
		"#####\n"
		"#@  #\n"
		"#   #\n"
		"#$ .#\n"
		"#####";

	fillState(s, mapStr);
	ASSERT_EQ(5, s.getWidth());
	ASSERT_EQ(5, s.getHeight());
	ASSERT_FALSE(s.canMove(1, 1)) << "1,1 should not be moveable";
	ASSERT_FALSE(s.canMove(1, 2)) << "1,2 should not be moveable - there is a package below";
	ASSERT_FALSE(s.canMove(1, 3)) << "1,3 should not be moveable";
	ASSERT_TRUE(s.canMove(2, 1)) << "2,1 should be moveable";
	ASSERT_TRUE(s.canMove(2, 2)) << "2,2 should be moveable";
	ASSERT_FALSE(s.canMove(2, 3)) << "2,3 should not be moveable - there is a package on the left";
	ASSERT_FALSE(s.canMove(3, 1)) << "3,1 should not be moveable";
	ASSERT_TRUE(s.canMove(3, 2)) << "3,2 should be moveable";
	// even though this is a target - we still can't move the package anymore
	ASSERT_FALSE(s.canMove(3, 3)) << "3,3 should not be moveable - even though this is a target";

	ASSERT_TRUE(s.hasDeadlock()) << "there is a deadlock that is not detected in 1,3";
}

}