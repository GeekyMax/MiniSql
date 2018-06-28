cmake_minimum_required(VERSION 3.10)
project(minisql)

set(CMAKE_CXX_STANDARD 11)

add_executable(
        minisql
        main.cpp

        BufferManager.cpp
        BPlusTree.cpp
        IndexManager.cpp

        BufferManager.h
        BPlusTree.h
        IndexManager.h
        CatalogManager.h
        CatalogManager.cpp
        Condition.h
        Condition.cpp
        IndexInfo.h
        RecordManager.h
        RecordManager.cpp
        Minisql.h
        Interpreter.h
        Interpreter.cpp
        API.h
        API.cpp
        Attribute.h
        Attribute.cpp
)