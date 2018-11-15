#pragma region project includes
#include "PlayerPawn.h"
#include "Component/SettingsComponent.h"
#include "Component/LessonsComponent.h"
#include "Game/LTVREGameInstance.h"
#include "Lesson/LocationObjectGroup.h"
#include "Lesson/Component/LocationSingleObjectComponent.h"
#include "UI/QuestionBase.h"
#include "UI/Interaction.h"
#pragma endregion

#pragma region UE4 includes
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextRenderComponent.h"
#pragma endregion

#pragma region constructor
// constructor
APlayerPawn::APlayerPawn()
{
	// create root scene component
	USceneComponent* pRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = pRoot;

	// create default camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(pRoot);

	// create default static mesh component
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(Camera);
	HeadMesh->SetVisibility(false);
	HeadMesh->SetOwnerNoSee(true);
	HeadMesh->SetCollisionProfileName("NoCollision");

	// create default widget interaction component
	WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction"));
	WidgetInteraction->SetupAttachment(Camera);

	// create default text render component
	NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
	NameText->SetupAttachment(pRoot);
	NameText->SetOwnerNoSee(true);

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

#pragma region public override function
// called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if map is menu or not local player return
	if (GetWorld()->GetMapName() == "Menu" || !IsLocallyControlled())
		return;

	// hit result
	FHitResult hit;

	// trace from camera forward
	GetWorld()->LineTraceSingleByChannel(
		hit,
		Camera->GetComponentLocation(),
		Camera->GetComponentLocation() + Camera->GetForwardVector() * 10000,
		ECollisionChannel::ECC_Visibility
	);

	// save player status
	EPlayerStatus status = ((ULTVREGameInstance*)GetGameInstance())->GetPlayerStatus();

	// save trace target to id (0 = no valid target, 1 = question widget target, 2 = single object target)
	int targetID = 0;
	
	// if hit component valid and tag correct
	if (hit.Component.IsValid() &&
		((hit.Component->ComponentHasTag("QuestionPractice") && status == EPlayerStatus::PRACTICE) ||
		(hit.Component->ComponentHasTag("QuestionTeacher") && status == EPlayerStatus::TEACHER) ||
		(hit.Component->ComponentHasTag("QuestionStudent") && status == EPlayerStatus::STUDENT)))
	{
		targetID = 1;
	}
	
	// if hit actor valid and tag correct
	else if (hit.Actor.IsValid() && hit.Actor->ActorHasTag("LessonObject"))
	{
		targetID = 2;
	}

	// if target id valid
	if (targetID)
	{
		// set trace target
		m_pTraceTarget = hit.GetActor();

		// if target id is question widget target
		if(targetID == 1)
			// release widget interaction click
			WidgetInteraction->ReleasePointerKey(FKey("LeftMouseButton"));

		// increase click timer
		m_clickTimer += DeltaTime;

		// if click timer lower than time to click return
		if (m_clickTimer >= ClickTime)
		{
			// if target id is question widget target
			if (targetID == 1)
			{
				// click widget interaction
				WidgetInteraction->PressPointerKey(FKey("LeftMouseButton"));

				// if not practice click on widget
				if (status != EPlayerStatus::PRACTICE)
				{
					// get question base from hit component
					UQuestionBase* pQuestionBase = (UQuestionBase*)(((UWidgetComponent*)(hit.GetComponent()))->GetUserWidgetObject());

					// click at question base widget
					pQuestionBase->ClickOnWidget(((UWidgetComponent*)(hit.GetComponent()))->GetDrawSize(),
						hit.GetComponent()->GetComponentTransform(), hit.Location);
				}
			}

			// if target id is single object target
			else
			{
				// toggle visibility of question widget
				((ASingleObject*)(hit.GetActor()))->ToggleQuestionWidget(status);
			}

			// reset click timer
			m_clickTimer = 0.0f;
		}
	}

	// if trace target valid
	else if(m_pTraceTarget != nullptr)
	{
		// set trace target null and click timer 0
		m_pTraceTarget = nullptr;
		m_clickTimer = 0.0f;

		// set percentage 0 and set image percentage
		m_pInteraction->SetPercentage(0.0f);
		m_pInteraction->SetImagePercentage();
	}

	// if interaction reference valid
	if (m_pInteraction)
	{
		// set percentage of interaction widget
		m_pInteraction->SetPercentage(m_clickTimer / ClickTime * 100.0f);

		// set image percentage of interaction widget
		m_pInteraction->SetImagePercentage();
	}

	// if server set camera rotation on clients
	if (HasAuthority())
		SetCameraRotationClient(Camera->RelativeRotation);

	// if client set camera rotation on server
	else
		SetCameraRotationServer(Camera->RelativeRotation);

}
#pragma endregion

#pragma region UFUNCTION
// initialize lesson in vr level
void APlayerPawn::InitializeLesson()
{
	// if not local player return
	if (!IsLocallyControlled())
		return;

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
									pObjGrp = GetWorld()->SpawnActor<ASingleObject>(objClass, pActor->GetActorLocation(), 
										pActor->GetActorRotation());
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
										ASingleObject* pSingleObj = GetWorld()->SpawnActor<ASingleObject>(singleObjClass, 
											((USceneComponent*)pLocSingleObjComp)->GetComponentLocation(),
											((USceneComponent*)pLocSingleObjComp)->GetComponentRotation());

										// check all questions
										for (FLessonObject lessonObj : Lessons->GetAllQuestions())
										{
											// if current question is equal with current object question
											if (lessonObj.Name == objGrp.Objects[j].QuestionName)
											{
												// set player reference of current object actor
												pSingleObj->SetPlayer(this);

												// set question of current object actor
												pSingleObj->SetLessonObject(lessonObj);

												// hide meshes
												pSingleObj->MeshesVisible = false;

												// question practice widget of object
												UQuestionBase* pQuestion = (UQuestionBase*)pSingleObj->QuestionPractice->GetUserWidgetObject();

												// initialize question practice widget
												InitWidget(pQuestion, pSingleObj);

												// question teacher widget of object
												pQuestion = (UQuestionBase*)pSingleObj->QuestionTeacher->GetUserWidgetObject();

												// initialize question teacher widget
												InitWidget(pQuestion, pSingleObj);
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

// set interaction reference
void APlayerPawn::SetInteraction(UInteraction* Interaction)
{
	// set interaction widget reference
	m_pInteraction = Interaction;

	// set percentage and image percentage
	m_pInteraction->SetPercentage(0.0f);
	m_pInteraction->SetImagePercentage();
}

// set rotation on server validate
bool APlayerPawn::SetCameraRotationServer_Validate(FRotator rotation)
{
	return true;
}

// set rotation on server implementation
void APlayerPawn::SetCameraRotationServer_Implementation(FRotator rotation)
{
	// set rotation on clients
	SetCameraRotationClient(rotation);
}

// set rotation on client implementation
void APlayerPawn::SetCameraRotationClient_Implementation(FRotator rotation)
{
	// if not local player
	if (!IsLocallyControlled())
		// set local rotation of camera
		Camera->SetRelativeRotation(rotation);
}

// set name text on server validate
bool APlayerPawn::SetNameTextServer_Validate(const FString& _name)
{
	return true;
}

// set name text on server implementation
void APlayerPawn::SetNameTextServer_Implementation(const FString& _name)
{
	// show head mesh
	HeadMesh->SetVisibility(true);

	// set text of name text
	NameText->SetText(FText::FromString(_name));

	// array to save all players into
	TArray<AActor*> pFoundActors;

	// get all players and save it to the array
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerPawn::StaticClass(), pFoundActors);

	// check all player
	for (AActor* pPlayer : pFoundActors)
	{
		// if local player
		if (((APlayerPawn*)pPlayer)->IsLocallyControlled())
		{
			// rotate name text to server player
			NameText->SetWorldRotation((pPlayer->GetActorLocation() - NameText->GetComponentLocation()).Rotation());

			// show head mesh on clients
			((APlayerPawn*)pPlayer)->ShowTeacherComponentsClient(((APlayerPawn*)pPlayer)->Settings->GetName());
		}
	}
}

// show head mesh and name text on clients implementation
void APlayerPawn::ShowTeacherComponentsClient_Implementation(const FString& _name)
{
	// show head mesh
	HeadMesh->SetVisibility(true);

	// set name text
	NameText->SetText(FText::FromString(_name));
}
#pragma endregion

#pragma region protected override function
// called at begin play
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	// reset name text
	NameText->SetText(FText::FromString(""));

	// if map is menu return
	if (GetWorld()->GetMapName() == "Menu")
		return;

	// if client and local player set name text
	if (!HasAuthority() && IsLocallyControlled())
		SetNameTextServer(Settings->GetName());
}
#pragma endregion

#pragma region private function
// initialize question widget
void APlayerPawn::InitWidget(UQuestionBase* _pWidget, ASingleObject* _pSingleObj)
{
	// set player reference of widget
	_pWidget->SetPlayer(this);

	// set object reference of widget
	_pWidget->SetObject(_pSingleObj);

	// get references
	_pWidget->GetReferences();

	// set question of question practice
	_pWidget->SetQuestion(_pSingleObj->GetLessonObjectQuestion());

	// set notice of question practice
	_pWidget->SetNotice(_pSingleObj->GetLessonObjectNotice());

	// set answers of question practice
	_pWidget->SetAnswerTexts(_pSingleObj->GetLessonObjectAnswers(),	_pSingleObj->GetCorrectAnswer());
}
#pragma endregion