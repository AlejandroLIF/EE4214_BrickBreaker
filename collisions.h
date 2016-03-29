typedef enum{
	COLLIDE_NONE,
    COLLIDE_BAR_AMINUS,
    COLLIDE_BAR_SMINUS,
    COLLIDE_BAR_N,
    COLLIDE_BAR_SPLUS,
    COLLIDE_BAR_APLUS,
    COLLIDE_BRICK_UL,
    COLLIDE_BRICK_UC,
    COLLIDE_BRICK_UR,
    COLLIDE_BRICK_CL,
    COLLIDE_BRICK_CR,
    COLLIDE_BRICK_BL,
    COLLIDE_BRICK_BC,
    COLLIDE_BRICK_BR,
    COLLIDE_WALL_LEFT,
    COLLIDE_WALL_RIGHT,
    COLLIDE_WALL_CEIL,
    COLLIDE_WALL_FLOOR
} CollisionCode;

CollisionCode checkCollideBar(Ball* ball, Bar* bar);
CollisionCode checkCollideWall(Ball* ball);
CollisionCode checkCollideBrick(Ball* ball, Brick* brick);