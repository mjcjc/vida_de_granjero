#pragma once
#include"Struct.pb.h"


class Player
{
public:
	uint16					connect = 0;
	uint64					playerId = 0;
	string					name;
	/*transformPosit			ts;*/
	Protocol::Transform		ts;
	Protocol::PlayerType	type = Protocol::PLAYER_TYPE_NONE;
	Protocol::PlayerState   playerState = Protocol::IDLE;
	GameSessionRef			ownerSession; // Cycle
	uint64                  lastMoveTimeMs = 0;
	uint32                  invalidMoveCount = 0;
	uint32                  lastProcessedMoveSequence = 0;
};
