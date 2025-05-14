#include "Unit.h"
#include "Game.h"


const float Unit::baseSpeed = 0.5f;
const float Unit::size = 0.48f;




Unit::Unit(SDL_Renderer* renderer, Vector2D setPos, int roundNumber) :
	pos(setPos), timerJustHurt(0.25f) {
	texture = TextureLoader::loadTexture(renderer, "Unit.bmp");
	
	// Increase speed by 50% each round
	currentSpeed = baseSpeed * (1.0f + (roundNumber * 0.5f));
}



void Unit::update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits) {
	timerJustHurt.countDown(dT);

	//Determine the distance to the target from the unit's current position.
	float distanceToTarget = (level.getTargetPos() - pos).magnitude();

	if (distanceToTarget < 0.5f) {
		healthCurrent = 0;
		hasReachedTarget = true;
	}
	else {
		//Determine the distance to move this frame.
		float distanceMove = currentSpeed * dT;
		if (distanceMove > distanceToTarget)
			distanceMove = distanceToTarget;

		//Find the normal from the flow field.
		Vector2D directionNormal(level.getFlowNormal((int)pos.x, (int)pos.y));
		//If this reached the target tile, then modify directionNormal to point to the target tile.
		if ((int)pos.x == (int)level.getTargetPos().x && (int)pos.y == (int)level.getTargetPos().y)
			directionNormal = (level.getTargetPos() - pos).normalize();

		Vector2D posAdd = directionNormal * distanceMove;

		//Check if the new position would overlap any other units or not.
		bool moveOk = true;
		for (int count = 0; count < listUnits.size() && moveOk; count++) {
			auto& unitSelected = listUnits[count];
			if (unitSelected != nullptr && unitSelected.get() != this &&
				unitSelected->checkOverlap(pos, size)) {
				//They overlap so check and see if this unit is moving towards or away 
				//from the unit it overlaps.
				Vector2D directionToOther = (unitSelected->pos - pos);
				//Ensure that they're not directly on top of each other.
				if (directionToOther.magnitude() > 0.01f) {
					//Check the angle between the units positions and the direction that this unit 
					//is traveling.  Ensure that this unit isn't moving directly towards the other 
					//unit (by checking the angle between).
					Vector2D normalToOther(directionToOther.normalize());
					float angleBtw = abs(normalToOther.angleBetween(directionNormal));
					if (angleBtw < 3.14159265359f / 4.0f)
						//Don't allow the move.
						moveOk = false;
				}
			}
		}

		if (moveOk) {
			//Check if it needs to move in the x direction.  If so then check if the new x position, plus an amount of spacing 
			//(to keep from moving too close to the wall) is within a wall or not and update the position as required.
			const float spacing = 0.35f;
			int x = (int)(pos.x + posAdd.x + copysign(spacing, posAdd.x));
			int y = (int)(pos.y);
			if (posAdd.x != 0.0f && level.isTileWall(x, y) == false)
				pos.x += posAdd.x;

			//Do the same for the y direction.
			x = (int)(pos.x);
			y = (int)(pos.y + posAdd.y + copysign(spacing, posAdd.y));
			if (posAdd.y != 0.0f && level.isTileWall(x, y) == false)
				pos.y += posAdd.y;
		}
	}
}



void Unit::draw(SDL_Renderer* renderer, int tileSize) {
	if (renderer != nullptr) {
		//Set the texture's draw color to red if this unit was hurt recently.
		if (timerJustHurt.timeSIsZero() == false)
			SDL_SetTextureColorMod(texture, 255, 0, 0);
		else
			SDL_SetTextureColorMod(texture, 255, 255, 255);

		//Draw the image at the unit's position.
		int w, h;
		SDL_QueryTexture(texture, NULL, NULL, &w, &h);
		SDL_Rect rect = {
			(int)(pos.x * tileSize) - w / 2,
			(int)(pos.y * tileSize) - h / 2,
			w,
			h };
		SDL_RenderCopy(renderer, texture, NULL, &rect);
	}
}



bool Unit::checkOverlap(Vector2D posOther, float sizeOther) {
	return (posOther - pos).magnitude() <= (sizeOther + size) / 2.0f;
}



bool Unit::isAlive() {
	return (healthCurrent > 0);
}



Vector2D Unit::getPos() {
	return pos;
}



void Unit::removeHealth(int damage) {
	if (damage > 0) {
		healthCurrent -= damage;
		if (healthCurrent < 0)
			healthCurrent = 0;

		timerJustHurt.resetToMax();
	}
}