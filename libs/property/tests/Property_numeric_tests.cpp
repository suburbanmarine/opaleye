#include "property/Property_numeric.hpp"
#include "property/Property_factory.hpp"

#include "gtest/gtest.h"

TEST(Property_numeric_i8, construct)
{
	Property_numeric_i8 foo;
}


TEST(Property_numeric_i8, to_json)
{
	Property_numeric_i8 foo;

	std::string str = foo.to_json();
}

TEST(Property_numeric_i8, metadata_to_json)
{
	Property_numeric_i8 foo;

	std::string str = foo.metadata_to_json();
}

TEST(Property_numeric_i8, from_str_valid)
{
	const std::string foo_json = 
R"({
		"name": "foo",
		"value": "10",
		"type": "i8",
		"constraints": {
			"min": "5",
			"max": "15"
		}
	}
)";

	std::shared_ptr<Property_base> foo = Property_factory::from_json(foo_json);
	ASSERT_NE(foo, nullptr);

	std::shared_ptr<Property_numeric_i8> bar = std::dynamic_pointer_cast<Property_numeric_i8>(foo);
	ASSERT_NE(bar, nullptr);

	EXPECT_EQ(bar->min(), 5);
	EXPECT_EQ(bar->max(), 15);
	EXPECT_EQ(bar->value(), 10);

	EXPECT_TRUE(bar->is_value_valid());
}

TEST(Property_numeric_i8, from_str_out_of_range)
{
	const std::string foo_json = 
R"({
		"name": "foo",
		"value": "25",
		"type": "i8",
		"constraints": {
			"min": "5",
			"max": "15"
		}
	}
)";

	std::shared_ptr<Property_base> foo = Property_factory::from_json(foo_json);
	ASSERT_NE(foo, nullptr);

	std::shared_ptr<Property_numeric_i8> bar = std::dynamic_pointer_cast<Property_numeric_i8>(foo);
	ASSERT_NE(bar, nullptr);

	EXPECT_EQ(bar->min(), 5);
	EXPECT_EQ(bar->max(), 15);
	EXPECT_EQ(bar->value(), 25);

	EXPECT_FALSE(bar->is_value_valid());
}
