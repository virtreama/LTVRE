#pragma once

// project includes
#include "Utility/Enums.h"

// UE4 includes
#include "Runtime/Engine/Classes/Engine/SkeletalMesh.h"

// system includes
#include <list>
#include <map>

// usings
using namespace std;

/// <summary>
/// single object from a lesson
/// </summary>
struct FLessonObject
{
	/// <summary>
	/// name of object
	/// </summary>
	FString m_Name;

	/// <summary>
	/// transform of object relative to group
	/// </summary>
	FTransform m_Transform;

	/// <summary>
	/// picture of object
	/// </summary>
	UTexture2D* m_PPicture;

	/// <summary>
	/// static mesh of object
	/// </summary>
	UStaticMesh* m_PStaticMesh;

	/// <summary>
	/// skeletal mesh of object
	/// </summary>
	USkeletalMesh* m_PSkeletalMesh;

	/// <summary>
	/// notice to this object
	/// </summary>
	FString m_Notice;

	/// <summary>
	/// question to this object
	/// </summary>
	FString m_Question;

	/// <summary>
	/// answers for the question
	/// </summary>
	list<FString> Answers;
};

/// <summary>
/// group of objects from a lesson
/// </summary>
struct FLessonObjectGroup
{
	/// <summary>
	/// name of object group
	/// </summary>
	FString m_Name;

	/// <summary>
	/// picture of object group
	/// </summary>
	UTexture2D* m_PPicture;

	/// <summary>
	/// objects
	/// </summary>
	list<FLessonObject> m_Objects;
};

/// <summary>
/// map from a lesson
/// </summary>
struct FLessonMap
{
	/// <summary>
	/// type of map
	/// </summary>
	ELessonMap m_Map;

	/// <summary>
	/// picture of map
	/// </summary>
	UTexture2D* m_PPicture;

	/// <summary>
	/// transform for object groups 2D
	/// </summary>
	map<int, FTransform> m_Transform2D;

	/// <summary>
	/// object groups to id
	/// </summary>
	map<int, FLessonObjectGroup> m_ObjectGroupID;
};

/// <summary>
/// lesson
/// </summary>
struct FLesson
{
	/// <summary>
	/// name of lesson
	/// </summary>
	FString m_Name;

	/// <summary>
	/// availability of lesson
	/// </summary>
	ELessonAvailability m_Availability;

	/// <summary>
	/// category of lesson
	/// </summary>
	ELessonCategory m_Category;

	/// <summary>
	/// map of lesson
	/// </summary>
	FLessonMap m_Map;
};