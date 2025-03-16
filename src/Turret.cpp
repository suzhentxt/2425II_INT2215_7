#include "Turret.h"


const float Turret::speedAngular = MathAddon::angleDegToRad(180.0f), Turret::weaponRange = 5.0f;




Turret::Turret(SDL_Renderer* renderer, Vector2D setPos) :
	pos(setPos), angle(0.0f), timerWeapon(1.0f) {
	textureMain = TextureLoader::loadTexture(renderer, "Turret.bmp");
	textureShadow = TextureLoader::loadTexture(renderer, "Turret Shadow.bmp");
	mix_ChunkShoot = SoundLoader::loadSound("Turret Shoot.ogg");
}



void Turret::update(SDL_Renderer* renderer, float dT, std::vector<std::shared_ptr<Unit>>& listUnits,
	std::vector<Projectile>& listProjectiles) {
	//Update timer.
	timerWeapon.countDown(dT);

	//Check if a target has been found but is no longer alive or is out of weapon range.
	if (auto unitTargetSP = unitTarget.lock()) {
		if (unitTargetSP->isAlive() == false ||
			(unitTargetSP->getPos() - pos).magnitude() > weaponRange) {
			//Then reset it.
			unitTarget.reset();
		}
	}
	
	//Find a target if needed.
	if (unitTarget.expired())
		unitTarget = findEnemyUnit(listUnits);

	//Update the angle and shoot a projectile if needed.
	if (updateAngle(dT))
		shootProjectile(renderer, listProjectiles);
}


bool Turret::updateAngle(float dT) {
	//Rotate towards the target unit if needed and output if it's pointing towards it or not.
	if (auto unitTargetSP = unitTarget.lock()) {
		//Determine the direction normal to the target.
		Vector2D directionNormalTarget = (unitTargetSP->getPos() - pos).normalize();

		//Determine the angle to the target.
		float angleToTarget = directionNormalTarget.angleBetween(Vector2D(angle));

		//Update the angle as required.
		//Determine the angle to move this frame.
		float angleMove = -copysign(speedAngular * dT, angleToTarget);
		if (abs(angleMove) > abs(angleToTarget)) {
			//It will point directly at it's target this frame.
			angle = directionNormalTarget.angle();
			return true;
		}
		else {
			//It won't reach it's target this frame.
			angle += angleMove;
		}
	}

	return false;
}


void Turret::shootProjectile(SDL_Renderer* renderer, std::vector<Projectile>& listProjectiles) {
	//Shoot a projectile towards the target unit if the weapon timer is ready.
	if (timerWeapon.timeSIsZero()) {
		listProjectiles.push_back(Projectile(renderer, pos, Vector2D(angle)));

		//Play the shoot sound.
		if (mix_ChunkShoot != nullptr)
			Mix_PlayChannel(-1, mix_ChunkShoot, 0);

		timerWeapon.resetToMax();
	}
}



void Turret::draw(SDL_Renderer* renderer, int tileSize) {
	drawTextureWithOffset(renderer, textureShadow, 5, tileSize);
	drawTextureWithOffset(renderer, textureMain, 0, tileSize);
}


void Turret::drawTextureWithOffset(SDL_Renderer* renderer, SDL_Texture* textureSelected,
	int offset, int tileSize) {
	if (renderer != nullptr && textureSelected != nullptr) {
		//Draw the image at the turret's position and angle and offset.
		int w, h;
		SDL_QueryTexture(textureSelected, NULL, NULL, &w, &h);
		SDL_Rect rect = {
			(int)(pos.x * tileSize) - w / 2 + offset,
			(int)(pos.y * tileSize) - h / 2 + offset,
			w,
			h };
		SDL_RenderCopyEx(renderer, textureSelected, NULL, &rect,
			MathAddon::angleRadToDeg(angle), NULL, SDL_FLIP_NONE);
	}
}



bool Turret::checkIfOnTile(int x, int y) {
	return ((int)pos.x == x && (int)pos.y == y);
}



std::weak_ptr<Unit> Turret::findEnemyUnit(std::vector<std::shared_ptr<Unit>>& listUnits) {
	//Find the closest enemy unit to this turret.
	std::weak_ptr<Unit> closestUnit;
	float distanceClosest = 0.0f;

	//Loop through the entire list of units.
	for (auto& unitSelected : listUnits) {
		//Ensure that the selected unit exists.
		if (unitSelected != nullptr) {
			//Calculate the distance to the selected unit.
			float distanceCurrent = (pos - unitSelected->getPos()).magnitude();
			//Check if the unit is within range, and no closest unit has been found or the 
			//selected unit is closer than the previous closest unit.
			if (distanceCurrent <= weaponRange &&
				(closestUnit.expired() || distanceCurrent < distanceClosest)) {
				//Then set the closest unit to the selected unit.
				closestUnit = unitSelected;
				distanceClosest = distanceCurrent;
			}
		}
	}

	return closestUnit;
}