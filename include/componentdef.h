#pragma once

#include <bitset>
#include <glm\glm.hpp>

/** The maximum number of entities in an entity-component-system group. */
#define ENTITY_MAX 1024

/** Guaranteed to be able to hold the maximum ID an entity can have. */
typedef uint32_t EntityInt;
typedef EntityInt Entity;

/**
* Entity IDs are split into three different groups:
* - Global: These IDs are shared by server and clients and must be the same for networking
* - Static: These IDs are known by the server and clients, but are not networked. Again, they must be the same
* - Local: These IDs are not known by both server and clients, and are not networked. They can unique only locally, but must not interfere with the above two types

* Therefore two ranges of IDs are needed, networked IDs which must be unique on the server and clients together, and non-networked IDs which can be unique on clients.
* These ID range starts are defined as follows. ENTITY_MAX applies to each range respectively.
*/

/** Local IDs run from this value to #SHARED_ID_RANGE_START, non-inclusive. 0 should be reserved as null entity. */
#define LOCAL_ID_RANGE_START 1
/** Shared IDs run from this value to EntityInt max. */
#define SHARED_ID_RANGE_START 4000000
/** The end of the shared ID range, non-inclusive. Must be less than EntityInt max */
#define SHARED_ID_RANGE_STOP 4294967290

/** The maximum number of component types in an entity-component-system group. */
#define COMPONENT_TYPE_MAX 16

typedef std::bitset<COMPONENT_TYPE_MAX> ComponentSignature;

typedef glm::vec3 Position3D;
typedef glm::vec3 Rotation3D;
typedef glm::vec3 Scale3D;

typedef uint16_t ComponentType;

// All 3D objects have a 3D position
typedef Position3D Position3DComponent;

// Not all 3D objects have a rotation and scale
struct Transform3DComponent
{
	Rotation3D	rotation;
	Scale3D		scale;
};