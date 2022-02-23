#include "type_enum.h"
#include "ec_alloc.h"
#include "ec_enum.h"

static void person_student_ctor(person* self) { self->tag = person_student; }

static void person_student_dtor(person* self) { (void)self; }

person* person_student_new(int grade)
{
    person* p = ec_newcd(person, person_student_ctor, person_student_dtor);
    p->u.person_student.grade = grade;
    return p;
}

static void person_teacher_ctor(person* self) { self->tag = person_teacher; }

static void person_teacher_dtor(person* self) { (void)self; }

person* person_teacher_new(int age)
{
    person* p = ec_newcd(person, person_teacher_ctor, person_teacher_dtor);
    p->u.person_teacher.age = age;
    return p;
}
