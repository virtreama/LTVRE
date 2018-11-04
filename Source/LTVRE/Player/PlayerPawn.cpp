#pragma region project includes
#include "PlayerPawn.h"
#include "Component/SettingsComponent.h"
#include "Component/LessonsComponent.h"
#include "Game/LTVREGameInstance.h"
#include "Lesson/LocationObjectGroup.h"
#include "Lesson/Component/LocationSingleObjectComponent.h"
#pragma endregion

#pragma region UE4 includes
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#pragma endregion

#pragma region constructor
// constructor
APlayerPawn::APlayerPawn()
{
	USceneComponent* pRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = pRoot;

	// create default camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(pRoot);

	// create default settings component
	Settings = CreateDefaultSubobject<USettingsComponent>(TEXT("Settings"));

	// create default lessons component
	Lessons = CreateDefaultSubobject<ULessonsComponent>(TEXT("Lessons"));

	// load settings from file
	Settings->LoadSettings();

	// load lessons from file
	Lessons->LoadLessons();
}
#pragma endregion

#pragma region UFUNCTION
// initialize lesson in vr level
void APlayerPawn::InitializeLesson()
{
	// get current lesson
	FLesson lesson = ((ULTVREGameInstance*)GetGameInstance())->GetCurrentLesson();

	// get all place object group actors
	TArray<AActor*> FoundPlaceObjectGroup;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALocationObjectGroup::StaticClass(), FoundPlaceObjectGroup);

	// if player status is practice or teacher initialize lesson
	if (((ULTVREGameInstance*)GetGameInstance())->GetPlayerStatus() == EPlayerStatus::PRACTICE ||
		((ULTVREGameInstance*)GetGameInstance())->GetPlayerStatus() == EPlayerStatus::TEACHER)
	{
		// check all object groups from map
		for (int i = 0; i < lesson.Map.ObjectGroups.Num(); i++)
		{
			// check all user generated object groups in lesson
			for (FLessonObjectGroup objGrp : Lessons->GetAllObjectGroups())
			{
				// if name of current user generated object group equals current group object from map
				if (objGrp.Name == lesson.Map.ObjectGroups[i])
				{
					// object group actor
					ASingleObject* pObjGrp = nullptr;

					// check all object group subclasses to spawn the right object
					for (TSubclassOf<ASingleObject> objClass : Lessons->ObjectGroupClasses)
					{
						// if name of current object class contains user generated object group object name
						if (objClass->GetName().Contains(objGrp.ObjectName))
						{
							// check all spawn places for object groups
							for (AActor* pActor : FoundPlaceObjectGroup)
							{
								// if id of spawn place is current index
								if (((ALocationObjectGroup*)(pActor))->ID == i)
								{
									// spawn object group
									pObjGrp = GetWorld()->SpawnActor<ASingleObject>(objClass, pActor->GetActorLocation(), pActor->GetActorRotation());
								}
							}
						}
					}

					// if object group actor not valid break
					if (!pObjGrp)
						break;

					// check all objects of object group
					for (int j = 0; j < objGrp.Objects.Num(); j++)
					{
						// check all single object subclasses
						for (TSubclassOf<ASingleObject> singleObjClass : Lessons->SingleObjectClasses)
						{
							// if current single object class contains current object group object name
							if (singleObjClass->GetName().Contains(objGrp.Objects[j].Name))
							{
								// check current object group actor children
								for (UActorComponent* pLocSingleObjComp : pObjGrp->GetComponentsByClass(ULocationSingleObjectComponent::StaticClass()))
								{
									// if index of current location equals index of current object
									if (((ULocationSingleObjectComponent*)(pLocSingleObjComp))->ID == j)
									{
										// spawn object
										ASingleObject* singleObj = GetWorld()->SpawnActor<ASingleObject>(singleObjClass, 
											((USceneComponent*)pLocSingleObjComp)->GetComponentLocation(),
											((USceneComponent*)pLocSingleObjComp)->GetComponentRotation());

										// check all questions
										for (FLessonObject lessonObj : Lessons->GetAllQuestions())
										{
											// if current question is equal with current object question
											if (lessonObj.Name == objGrp.Objects[j].QuestionName)
											{
												// set question of current object actor
												singleObj->SetLessonObject(lessonObj);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
#pragma endregion