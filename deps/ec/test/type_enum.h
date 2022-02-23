#ifndef TYPE_ENUM_H
#define TYPE_ENUM_H

#include "ec_enum.h"

ect_enum_declare(person, person_student, person_teacher);

ect_enum_declare_begin(person) struct
{
    int grade;
} person_student;
struct
{
    int age;
} person_teacher;
ect_enum_declare_end();

person* person_student_new(int grade);
person* person_teacher_new(int age);

#endif
