#include "test_enum.h"
#include "testfw.h"
#include "type_enum.h"
#include <stdio.h>

void test_enum(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : enum");
    {
        /* new & delete */
        person* p = person_student_new(3);
        CUNITTEST_ASSERT_NE(p, NULL);
        ec_delete(p);
    }
    {
        /* if */
    } {
        /* match */
    }
    CUNITTEST_RESULT();
}
