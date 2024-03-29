#pragma region project includes
#include "LessonsComponent.h"
#include "Player/PlayerPawn.h"
#include "Helper/Helper.h"
#include "Helper/LessonParser.h"
#pragma endregion

#pragma region UE4 includes
#include "Engine/Texture2D.h"
#pragma endregion

#pragma region UFUNCTION
// set current lesson
void ULessonsComponent::SetCurrentLesson(FLesson Lesson, bool NewLesson)
{
	// if lesson is new set empty name
	if (NewLesson)
		m_currentLessonName = "";

	// if not new lesson set current lesson name
	else
		m_currentLessonName = Lesson.Name;

	// set current lesson
	m_currentLesson = Lesson;
}

// empty the current lesson to default
void ULessonsComponent::EmptyCurrentLesson()
{
	// reset lesson to default
	m_currentLesson = FLesson();

	// empty current object group name
	m_currentObjectGroupName = "";
}

// set name of current lesson
bool ULessonsComponent::SetLessonName(FString Name)
{
	// if name length is not valid return false
	if (Name.Len() <= 0)
		return false;

	// check all lessons
	for (FLesson lesson : m_lessons)
		// if name already in array
		if (lesson.Name == Name)
			// return false
			return false;

	// set name
	m_currentLesson.Name = "";

	// through all chars
	for (int i = 0; i < Name.Len(); i++)
	{
		// if char number more than 16 break for
		if (i > 15)
			break;

		// add char at index i
		m_currentLesson.Name += Name[i];
	}

	// return name changed true
	return true;
}

// set creator of current lesson
void ULessonsComponent::SetLessonCreator(FString Creator)
{
	m_currentLesson.Creator = Creator;
}

// set availability of current lesson
void ULessonsComponent::SetLessonAvailability(ELessonAvailability Availability)
{
	m_currentLesson.Availability = Availability;
}

// set category of current lesson
void ULessonsComponent::SetLessonCategory(ELessonCategory Category)
{
	m_currentLesson.Category = Category;
}

// set texture name of current lesson map texture
void ULessonsComponent::SetLessonMapPicture(FString TextureName)
{
	m_currentLesson.Map.Picture = TextureName;
}

// set object name of current lesson map at index
void ULessonsComponent::SetLessonMapObject(int Index, FString ObjectName)
{
	// while array length is lower than id add new object group name
	while (m_currentLesson.Map.ObjectGroups.Num() < Index + 1)
		m_currentLesson.Map.ObjectGroups.Add("");

	// set object group name at index
	m_currentLesson.Map.ObjectGroups[Index] = ObjectName;
}

// save current lesson to lessons list
bool ULessonsComponent::SaveCurrentLesson(bool LessonIsNew, int Index)
{
	// if current lesson has no name or index not valid return false
	if (m_currentLesson.Name.Len() <= 0 || (!LessonIsNew && (Index < 0 || Index > m_lessons.Num() - 1)))
		return false;

	// check all lessons
	for (FLesson lesson : m_lessons)
	{
		// if name already in array
		if (lesson.Name == m_currentLesson.Name &&
			m_currentLesson.Name != m_currentLessonName)
		{
			// empty current lesson
			EmptyCurrentLesson();

			// return false
			return false;
		}
	}

	// if current lesson is a new lesson
	if (LessonIsNew)
		// add current lesson to lesson array
		m_lessons.Add(m_currentLesson);

	// if current lesson is already in array
	else
		m_lessons[Index] = m_currentLesson;
	
	// save lessons
	SaveLesson();

	// return lesson saved true
	return true;
}

// delete current lesson at index
void ULessonsComponent::DeleteCurrentLessonAtIndex(int Index)
{
	// if invalid index return
	if (Index < 0 || Index > m_lessons.Num() - 1)
		return;

	// remove lesson at index
	m_lessons.RemoveAt(Index);

	// save lesson
	SaveLesson();
}

// set current lesson object group
void ULessonsComponent::SetCurrentObjectGroup(FLessonObjectGroup ObjectGroup, bool NewObjectGroup)
{
	// if object group is new set empty name
	if (NewObjectGroup)
		m_currentObjectGroupName = "";

	// if not new object group set current object group name
	else
		m_currentObjectGroupName = ObjectGroup.Name;

	// set current object group
	m_currentObjectGroup = ObjectGroup;
}

// empty the current object group to default
void ULessonsComponent::EmptyCurrentObjectGroup()
{
	// set current object group default lesson object group
	m_currentObjectGroup = FLessonObjectGroup();

	// if no object group textures return
	if (ObjectGroupTextures.Num() <= 0)
		return;

	// set current object group picture to first available
	m_currentObjectGroup.ObjectName = ObjectGroupTextures[0]->GetName();

	// empty current object group name
	m_currentObjectGroupName = "";
}

// set name of current object group
bool ULessonsComponent::SetObjectGroupName(FString Name)
{
	// if name length is not valid return false
	if (Name.Len() <= 0)
		return false;

	// check all object groups
	for (FLessonObjectGroup objGrp : m_objectGroups)
		// if name already in array
		if (objGrp.Name == Name)
			// return
			return false;

	// set name
	m_currentObjectGroup.Name = "";

	// through all chars
	for (int i = 0; i < Name.Len(); i++)
	{
		// if char number more than 16 break for
		if (i > 15)
			break;

		// add char at index i
		m_currentObjectGroup.Name += Name[i];
	}

	// return changed name true
	return true;
}

// set picture name of current object group
void ULessonsComponent::SetObjectGroupObjectName(FString Name)
{
	// if name length is not valid return
	if (Name.Len() <= 0)
		return;

	// set name
	m_currentObjectGroup.ObjectName = Name;
}

// set object group object at index (ID)
void ULessonsComponent::SetObjectGroupObject(int ID, FString LessonObjectName)
{
	// while array length is lower than id add new lesson object
	while (m_currentObjectGroup.Objects.Num() < ID + 1)
		m_currentObjectGroup.Objects.Add(FObjectGroupObject());

	// object group object to set
	FObjectGroupObject objGrpObj = FObjectGroupObject();

	// set name of object group object
	objGrpObj.Name = LessonObjectName;

	// set lesson object at index
	m_currentObjectGroup.Objects[ID] = objGrpObj;
}

// save current object group to object group list
bool ULessonsComponent::SaveCurrentObjectGroup()
{
	// if current object group has no name return false
	if (m_currentObjectGroup.Name.Len() <= 0)
		return false;

	// check all object groups
	for (FLessonObjectGroup objGrp : m_objectGroups)
	{
		// if name already in array
		if (objGrp.Name == m_currentObjectGroup.Name &&
			m_currentObjectGroup.Name != m_currentObjectGroupName)
		{
			// empty current object group
			EmptyCurrentObjectGroup();
			
			// return
			return false;
		}
	}

	// if current object group name is empty
	if (m_currentObjectGroupName == "")
	{
		// add current object group to array
		m_objectGroups.Add(m_currentObjectGroup);
	}

	//if current object group name valid
	else
	{
		// check all object groups, if name equals replace object group
		for (int i = 0; i < m_objectGroups.Num(); i++)
			if (m_objectGroups[i].Name == m_currentObjectGroupName)
				m_objectGroups[i] = m_currentObjectGroup;
	}
	
	// empty current object group
	EmptyCurrentObjectGroup();

	// save lessons
	SaveLesson();

	// return saved true
	return true;
}

// delete an object group at given index
void ULessonsComponent::DeleteObjectGroupAtIndex(int Index)
{
	m_objectGroups.RemoveAt(Index);
}

// set current question
void ULessonsComponent::SetCurrentQuestion(FLessonObject Question)
{
	m_currentQuestion = Question;
}

// set notice of current question
bool ULessonsComponent::SetCurrentQuestionNotice(FString Notice)
{
	// text to set
	FString text = "";

	// check all chars of notice
	for (int i = 0; i < Notice.Len(); i++)
		// if count under 32 add char to text
		if (i < 31)
			text += Notice[i];

	// set notice of current question
	m_currentQuestion.Notice = text;

	// save current question and return if saved
	return SaveCurrentQuestion();
}

// set question o current question
bool ULessonsComponent::SetCurrentQuestionQuestion(FString Question)
{
	// text to set
	FString text = "";

	// check all chars of notice
	for (int i = 0; i < Question.Len(); i++)
		// if count under 32 add char to text
		if (i < 31)
			text += Question[i];

	// set notice of current question
	m_currentQuestion.Question = text;

	// save current question
	return SaveCurrentQuestion();
}

// set answer at index of current question
bool ULessonsComponent::SetCurrentQuestionAnswer(int Index, FString Answer)
{
	// while array length is lower than id add new answer
	while (m_currentQuestion.Answers.Num() < Index + 1)
		m_currentQuestion.Answers.Add("");

	// text to set
	FString text = "";

	// check all chars of answer
	for (int i = 0; i < Answer.Len(); i++)
		// if count under 32 add char to text
		if (i < 31)
			text += Answer[i];

	// set answer of current question
	m_currentQuestion.Answers[Index] = text;

	// save current question
	return SaveCurrentQuestion();
}

// delete current question from array
void ULessonsComponent::DeleteCurrentQuestion()
{
	// index to delete question from catalog
	int indexToDelete = -1;

	// check all questions
	for (int i = 0; i < m_questionCatalog.Num(); i++)
		// if current question name is equal to question in catalog
		if (m_questionCatalog[i].Name == m_currentQuestion.Name)
			// set index to delete to iterator index
			indexToDelete = i;

	// if index to delete not found return
	if (indexToDelete == -1)
		return;

	// delete question from catalog
	m_questionCatalog.RemoveAt(indexToDelete);

	// name questions new
	for (int i = 0; i < m_questionCatalog.Num(); i++)
	{
		// set name of question
		m_questionCatalog[i].Name = "Question";
		m_questionCatalog[i].Name.Append(FString::FromInt(i));
	}

	// save to file
	SaveLesson();
}
#pragma endregion

#pragma region public function
// load Lessons.xml
void ULessonsComponent::LoadLessons()
{
	// file to lesson parser
	FileToLessonParser ftlp;

	// error saves
	FText errorText = FText::FromString("");
	int errorNumber = 0;

	// parse xml file into file to string parser
	if (!FFastXml::ParseXmlFile(&ftlp, *Helper::GetAbsoluteFileName("Lessons.xml"),
		nullptr, nullptr, false, false, errorText, errorNumber))
	{
		// log errors
		GLog->Log("LTVRE", ELogVerbosity::Error, FString("Error: ").Append(errorText.ToString()));
		GLog->Log("LTVRE", ELogVerbosity::Error, FString("Error number: ") + FString::FromInt(errorNumber));
	}

	// set arrays from parser to component
	m_questionCatalog = ftlp.m_LessonObjects;
	m_objectGroups = ftlp.m_LessonObjectGroups;
	m_lessons = ftlp.m_Lessons;
}

// add lesson
void ULessonsComponent::AddLesson(FLesson _lesson)
{
	// if lesson is not in list
	bool isNewLesson = true;

	// check all lessons
	for (int i = 0; i < m_lessons.Num(); i++)
	{
		// if name of current lesson equal with given
		if (m_lessons[i].Name == _lesson.Name)
		{
			// update current lesson
			m_lessons[i] = _lesson;

			// new lesson false
			isNewLesson = false;

			// break for
			break;
		}
	}

	// if lesson new add lesson
	if(isNewLesson)
		m_lessons.Add(_lesson);

	// save lessons
	SaveLesson();
}

// add question to result lesson
void ULessonsComponent::AddResultLessonQuestion(FLessonObject _lessonObj)
{
	// add new result question
	m_lessonResult.m_Questions.Add(FResultQuestion());

	// set lesson object of added result question
	m_lessonResult.m_Questions[m_lessonResult.m_Questions.Num() - 1].m_Question = _lessonObj;
}

// add student to result lesson
void ULessonsComponent::AddResultLessonStudent(FString _name)
{
	// check all result questions
	for (int i = 0; i < m_lessonResult.m_Questions.Num(); i++)
	{
		// add new student and answer
		m_lessonResult.m_Questions[i].m_StudentsAnswer.Add(FResultStudentAnswer());

		// set name of added student
		m_lessonResult.m_Questions[i].m_StudentsAnswer[m_lessonResult.m_Questions[i].m_StudentsAnswer.Num() - 1].m_Name = _name;
	}
}

// set given answer of student in result lesson
void ULessonsComponent::SetResultQuestionGivenAnswer(FString _studentName, FString _objName, FString _answer)
{
	// check all result questions
	for (int i = 0; i < m_lessonResult.m_Questions.Num(); i++)
		// if question object equal answered object
		if (m_lessonResult.m_Questions[i].m_Question.ObjectName == _objName)
			// check all student answers of question
			for (int j = 0; j < m_lessonResult.m_Questions[i].m_StudentsAnswer.Num(); j++)
				// if name of student is equal with given name
				if (m_lessonResult.m_Questions[i].m_StudentsAnswer[j].m_Name == _studentName)
					// set answer of current student
					m_lessonResult.m_Questions[i].m_StudentsAnswer[j].m_GivenAnswer = _answer;
}

// save result of lesson
void ULessonsComponent::SaveLessonResult()
{
	// file to write into
	ofstream file;

	// open file
	file.open(TCHAR_TO_ANSI(*Helper::GetAbsoluteFileName(FString(m_lessonResult.m_LessonDate).Append(".xml"))));

	// if file could not be opened return
	if (!file.is_open())
		return;

	// write lesson result to file
	file << TCHAR_TO_ANSI(*Helper::StructToStringXML(m_lessonResult));

	// close file
	file.close();
}
#pragma endregion

#pragma region private function
// save lessons and object groups to file
void ULessonsComponent::SaveLesson()
{
	// file to write into
	ofstream file;

	// open file
	file.open(TCHAR_TO_ANSI(*Helper::GetAbsoluteFileName("Lessons.xml")));

	// if file could not be opened return
	if (!file.is_open())
		return;

	// check each question and write into string
	for (auto& Elem : m_questionCatalog)
		file << TCHAR_TO_ANSI(*Helper::StructToStringXML(Elem));

	// check each object group and write into string
	for (auto& Elem : m_objectGroups)
		file << TCHAR_TO_ANSI(*Helper::StructToStringXML(Elem));

	// check each lesson and write into string
	for (auto& Elem : m_lessons)
		file << TCHAR_TO_ANSI(*Helper::StructToStringXML(Elem));

	// close file
	file.close();
}

// save current question to array
bool ULessonsComponent::SaveCurrentQuestion()
{
	// if question length under 0 or answer 1 empty return
	if (m_currentQuestion.Question.Len() <= 0 ||
		(m_currentQuestion.Answers.Num() > 0 && m_currentQuestion.Answers[0] == ""))
		return false;

	// bool to check if current question already in catalog
	bool questionAlreadyInCatalog = false;

	// check all questions
	for (int i = 0; i < m_questionCatalog.Num(); i++)
	{
		// if current question name is equal to question in catalog
		if (m_questionCatalog[i].Name == m_currentQuestion.Name)
		{
			// if question length is under 0 continue
			if (m_currentQuestion.Question.Len() > 0)
			{
				// set question in catalog to current question
				m_questionCatalog[i] = m_currentQuestion;

				// set bool question already in catalog true
				questionAlreadyInCatalog = true;
			}
		}
	}

	// if question not already in catalog
	if (!questionAlreadyInCatalog)
	{
		// set current question name to question + count of catalog
		m_currentQuestion.Name = "Question";
		m_currentQuestion.Name.Append(FString::FromInt(m_questionCatalog.Num()));

		// add current question to catalog
		m_questionCatalog.Add(m_currentQuestion);
	}

	// save to file
	SaveLesson();

	// return question saved successfully
	return true;
}
#pragma endregion