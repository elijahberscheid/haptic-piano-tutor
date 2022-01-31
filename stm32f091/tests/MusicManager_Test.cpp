#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdlib.h>
#include <assert.h>
#include "../src/modules/GlobalVariables.h"
}

TEST_GROUP(MusicManager)
{
    int exampleVariable = 0;

    enum
    {
        Example_Enum = 1
    };

    void setup()
    {
    }

    void teardown()
    {
    }

};

TEST(MusicManager, ShouldRunTests)
{
    CHECK_EQUAL(1, 1);
}

