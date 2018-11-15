--[[
***********************************************************
jcr_testscript.lua
This particular file has been released in the public domain
and is therefore free of any restriction. You are allowed
to credit me as the original author, but this is not 
required.
This file was setup/modified in: 
2018
If the law of your country does not support the concept
of a product being released in the public domain, while
the original author is still alive, or if his death was
not longer than 70 years ago, you can deem this file
"(c) Jeroen Broks - licensed under the CC0 License",
with basically comes down to the same lack of
restriction the public domain offers. (YAY!)
*********************************************************** 
Version 18.11.03
]]
-- This lua script is solely used to quickly create a JCR file I can use to 
-- test JCR6 out in C
-- Also note that the folder setup has been set to match my own file system on my current working device (mac), so it may need adaption to work on yours (or when I switch to another OS).

fstorage = "Store"
estorage = "Store" 
comments = {
	Hello = "Hello World!",
	Lied = "Ozewiesewozewiesewassakristassa Kirstosewiesewosewiese wies wies wies wies!"
}

files = {
	["jcr6.c"]="/Users/Rachel/Projects/Scyndi/Applications/C/JCR6/jcr6.c",
	["jcr6.h"]="/Users/Rachel/Projects/Scyndi/Applications/C/JCR6/jcr6.h",
	["ReadMe.md"]="/Users/Rachel/Projects/Scyndi/Applications/C/JCR6/ReadMe.md",
	["jcr6_test.c"]="/Users/Rachel/Projects/Scyndi/Applications/C/JCR6/test/jcr6_test.c"
}


for k,c in pairs(comments) do
	--AddComment(k,c)
	JLS.Output("COMMENT:"..k..","..c)
end


i=0
for k,f in pairs(files) do
	i=i+1
	Add(f,k,{Author="Jeroen", Notes="File #"..i,Store=estorage})
end

Alias("jcr6.h","stomme header.h")


output="/Users/Rachel/Projects/Scyndi/Applications/C/JCR6/test/test.jcr"

print("I'll output to: "..output)
JLS.SetJCR6OutputFile(output)
Sig("436e5391f23ab54e6ae3f8b0b3b7369a38b92cac")
JLS.Output("FATSTORAGE:"..fstorage)
 
