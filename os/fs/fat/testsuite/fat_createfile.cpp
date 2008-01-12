/*
 * Copyright (c) 2006
 * Nintendo Co., Ltd.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Nintendo makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

#include <new>
#include <errno.h>
#include <stdlib.h>
#include <es.h>
#include <es/handle.h>
#include <es/exception.h>
#include "vdisk.h"
#include "fatStream.h"

#define TEST(exp)                           \
    (void) ((exp) ||                        \
            (esPanic(__FILE__, __LINE__, "\nFailed test " #exp), 0))

static void TestFileSystem(Handle<IContext>  root)
{
    const char* nameList[] =
    {
        "ﾌｧｲﾙ",
        "ファイル",
        "a1",
        "a2",
        "a3",
        "a4",
        "a5",
        "a6",
        "a7",
        "a8",
        "a9",
        "a10",
        "a11",
        "a12",
        "a13",
        "a14",
        "a15",
        "a16",

        "@abc",
        "@",
        "_abc",
        "_",
        "`abc",
        "`",
        "~abc",
        "~",
        "+abc",
        "+",
        "=abc",
        "=",
        "-abc",
        "-",
        "1",
        "1a",
        "a",
        "abc_defgh",
        "abc-defgh",
        "bcd",
        "a.b.c",
        "a b c",
        "a                     b                     c",
        "^",
        "^",
        "!",
        "!abc",
        "#",
        "#abc",
        "$",
        "$abc",
        "%",
        "%abc",
        "&",
        "&abc",
        "{}",
        "{abc}",
        ";abc",
        ";",
        ";;;",
        ";;;abc",
        "[]",
        "[abc]",
        ",",
        "aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeee"
        "ffffffffffgggggggggghhhhhhhhhhiiiiiiiiiijjjjjjjjjj"
        "kkkkkkkkkkllllllllllmmmmmmmmmmnnnnnnnnnnoooooooooo"
        "ppppppppppqqqqqqqqqqrrrrrrrrrrsssssssssstttttttttt"
        "uuuuuuuuuuvvvvvvvvvvwwwwwwwwwwxxxxxxxxxxyyyyyyyyyy"
        "zzzzz", // 255
        "...a",
        "..opq",
        ".bcd",
        // ".e.f.g.",
        ".e.f.g",
        // "hij.",
        // "k.l.m.",
        "k.l.m",
        // "rst..",
        // "u..v..w..",
        "u..v..w",
        // "xyz..."
    };

    esReport("Create Files\n");

    int i;
    for (i = 0; i < sizeof(nameList)/sizeof(nameList[0]); ++i)
    {
        int len = strlen(nameList[i]);
        esReport("create \"%s\" (len %d)\n", nameList[i], len);

        Handle<IFile> file = root->lookup(nameList[i]);
        if (!file)
        {
            file = root->bind(nameList[i], 0);
            TEST(file->isFile());
            TEST(!file->isDirectory());
            TEST(file->canRead());
            TEST(file->canWrite());
            TEST(!file->isHidden());

            Handle<IStream>     stream;
            stream = file->getStream();
            long ret = stream->write("0123456789\n", 11);
            TEST(ret == 11);

            //stream->flush();
        }
        TEST(file);

        char created[512];
        file->getName(created, sizeof(created));
        TEST(strcmp(created, nameList[i]) == 0);
    }

    // create a file with the path.
    root->createSubcontext("dir");
    Handle<IFile> file = root->bind("dir/abc", 0);
    TEST(file);
    file = 0;

    // create the file that already exists.
    file = root->bind("abc", 0);
    TEST(file);
    file = 0;
    file = root->bind("abc", 0);
    TEST(!file);
    file = root->bind("dir", 0);
    TEST(!file);

    // check wrong name.
    const char* wrongNameList[] =
    {
//        "\\",
//        "/",
        "\"",
        "*",
        "<",
        ">",
        "|",
        ":",
        "?",
        "aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeee"
        "ffffffffffgggggggggghhhhhhhhhhiiiiiiiiiijjjjjjjjjj"
        "kkkkkkkkkkllllllllllmmmmmmmmmmnnnnnnnnnnoooooooooo"
        "ppppppppppqqqqqqqqqqrrrrrrrrrrsssssssssstttttttttt"
        "uuuuuuuuuuvvvvvvvvvvwwwwwwwwwwxxxxxxxxxxyyyyyyyyyy"
        "zzzzzz" // 256
    };

    for (i = 0; i < sizeof(wrongNameList)/sizeof(wrongNameList[0]); ++i)
    {
        int len = strlen(wrongNameList[i]);
        esReport("create \"%s\" (len %d)\n", wrongNameList[i], len);

        Handle<IFile> file = root->lookup(wrongNameList[i]);
        if (!file)
        {
            try
            {
                file = root->bind(wrongNameList[i], 0);
            }
            catch (SystemException<EACCES>)
            {
            }
            catch (SystemException<ENAMETOOLONG>)
            {
            }
        }
        TEST(!file);
    }
}

int main(void)
{
    IInterface* ns = 0;
    esInit(&ns);
    Handle<IContext> nameSpace(ns);

    Handle<IClassStore> classStore(nameSpace->lookup("class"));
    esRegisterFatFileSystemClass(classStore);

#ifdef __es__
    Handle<IStream> disk = nameSpace->lookup("device/floppy");
#else
    Handle<IStream> disk = new VDisk(static_cast<char*>("2hd.img"));
#endif
    long long diskSize;
    diskSize = disk->getSize();
    esReport("diskSize: %lld\n", diskSize);

    Handle<IFileSystem> fatFileSystem;
    long long freeSpace;
    long long totalSpace;

    esCreateInstance(CLSID_FatFileSystem, IID_IFileSystem,
                     reinterpret_cast<void**>(&fatFileSystem));
    fatFileSystem->mount(disk);
    fatFileSystem->format();
    fatFileSystem->getFreeSpace(freeSpace);
    fatFileSystem->getTotalSpace(totalSpace);
    esReport("Free space %lld, Total space %lld\n", freeSpace, totalSpace);
    {
        Handle<IContext> root;

        fatFileSystem->getRoot(reinterpret_cast<IContext**>(&root));
        TestFileSystem(root);
        fatFileSystem->getFreeSpace(freeSpace);
        fatFileSystem->getTotalSpace(totalSpace);
        esReport("Free space %lld, Total space %lld\n", freeSpace, totalSpace);
        esReport("\nChecking the file system...\n");
        TEST(fatFileSystem->checkDisk(false));
    }
    fatFileSystem->dismount();
    fatFileSystem = 0;

    esCreateInstance(CLSID_FatFileSystem, IID_IFileSystem,
                     reinterpret_cast<void**>(&fatFileSystem));
    fatFileSystem->mount(disk);
    fatFileSystem->getFreeSpace(freeSpace);
    fatFileSystem->getTotalSpace(totalSpace);
    esReport("Free space %lld, Total space %lld\n", freeSpace, totalSpace);
    esReport("\nChecking the file system...\n");
    TEST(fatFileSystem->checkDisk(false));
    fatFileSystem->dismount();
    fatFileSystem = 0;

    esReport("done.\n\n");
}
