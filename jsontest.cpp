#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <iostream>

int main()
{
	rapidjson::Document doc;
	doc.SetObject();

	doc.AddMember("test", 0, doc.GetAllocator());
	
	rapidjson::Value ext_ctrl_desc_array;
	ext_ctrl_desc_array.SetArray();
	ext_ctrl_desc_array.GetArray().Reserve(6, doc.GetAllocator());

	rapidjson::StringBuffer buf;
	buf.Clear();

	rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
	doc.Accept(writer);

	std::cout << buf.GetString() << std::endl;

	return 0;
}