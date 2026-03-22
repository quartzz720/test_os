#include "../include/efi.h"
#include "console.h"
#include "fs.h"

CHAR16 cwd[256] = L"\\";

void build_path(CHAR16* path, CHAR16* name, CHAR16* result) {
    if (name[0] == L'\\') {
        strcpy16(result, name);
        return;
    }
    UINTN base = strlen16(path);
    strcpy16(result, path);
    if (result[base - 1] != L'\\') result[base++] = L'\\';
    UINTN i = 0;
    while (name[i] && base < 255) { result[base++] = name[i++]; }
    result[base] = 0;
}

EFI_FILE_PROTOCOL* open_root(void) {
    EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID sfs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL* lip = NULL;
    ST->BootServices->HandleProtocol(IH, &lip_guid, (void**)&lip);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* sfs = NULL;
    ST->BootServices->HandleProtocol(lip->DeviceHandle, &sfs_guid, (void**)&sfs);
    EFI_FILE_PROTOCOL* root = NULL;
    sfs->OpenVolume(sfs, &root);
    return root;
}

void cmd_dir(void) {
    EFI_FILE_PROTOCOL* root = open_root();
    if (!root) { println(L"Error: cannot open volume"); return; }
    EFI_FILE_PROTOCOL* dir = NULL;
    EFI_STATUS s = root->Open(root, &dir, cwd, EFI_FILE_MODE_READ, 0);
    if (s != EFI_SUCCESS) { println(L"Error: cannot open directory"); return; }
    set_color(EFI_YELLOW, EFI_BGBLACK);
    print(L"Directory of "); println(cwd);
    set_color(EFI_WHITE, EFI_BGBLACK);
    UINT8 buf[512];
    while (1) {
        UINTN size = sizeof(buf);
        s = dir->Read(dir, &size, buf);
        if (s != EFI_SUCCESS || size == 0) break;
        EFI_FILE_INFO* info = (EFI_FILE_INFO*)buf;
        if (info->Attribute & EFI_FILE_DIRECTORY) {
            set_color(EFI_CYAN, EFI_BGBLACK); print(L"  [DIR]  ");
        } else {
            set_color(EFI_WHITE, EFI_BGBLACK); print(L"  [FILE] ");
        }
        print(info->FileName);
        print(L"  ("); print_uint(info->FileSize); println(L" bytes)");
    }
    set_color(EFI_WHITE, EFI_BGBLACK);
    dir->Close(dir); root->Close(root);
}

void cmd_cd(CHAR16* input) {
    CHAR16* path = input + 3;
    if (strlen16(path) == 0) { println(cwd); return; }
    if (streq(path, L"..")) {
        if (cwd[0] == L'\\' && cwd[1] == 0) return;
        INTN i = (INTN)strlen16(cwd) - 1;
        if (cwd[i] == L'\\') i--;
        while (i > 0 && cwd[i] != L'\\') i--;
        if (i == 0) { cwd[0] = L'\\'; cwd[1] = 0; } else { cwd[i] = 0; }
        return;
    }
    if (streq(path, L"\\") || streq(path, L"/")) {
        cwd[0] = L'\\'; cwd[1] = 0; return;
    }
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* dir  = NULL;
    EFI_STATUS s = root->Open(root, &dir, path, EFI_FILE_MODE_READ, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK);
        println(L"Error: directory not found");
        set_color(EFI_WHITE, EFI_BGBLACK);
        root->Close(root); return;
    }
    dir->Close(dir); root->Close(root);
    if (path[0] == L'\\') {
        UINTN j = 0;
        while (path[j]) { cwd[j] = path[j]; j++; } cwd[j] = 0;
    } else {
        UINTN base = strlen16(cwd);
        if (cwd[base-1] != L'\\') cwd[base++] = L'\\';
        UINTN j = 0;
        while (path[j]) { cwd[base+j] = path[j]; j++; } cwd[base+j] = 0;
    }
}

void cmd_type(CHAR16* input) {
    CHAR16* name = input + 5;
    if (strlen16(name) == 0) { println(L"Usage: type [filename]"); return; }
    CHAR16 fullpath[256];
    build_path(cwd, name, fullpath);
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* file = NULL;
    EFI_STATUS s = root->Open(root, &file, fullpath, EFI_FILE_MODE_READ, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: file not found");
        set_color(EFI_WHITE, EFI_BGBLACK); return;
    }
    UINT8 buf[512];
    CHAR16 out[2] = {0, 0};
    while (1) {
        UINTN size = sizeof(buf);
        file->Read(file, &size, buf);
        if (size == 0) break;
        for (UINTN i = 0; i < size; i++) {
            if (buf[i] == '\n') { print(L"\r\n"); continue; }
            if (buf[i] == '\r') continue;
            out[0] = (CHAR16)buf[i]; print(out);
        }
    }
    println(L""); file->Close(file); root->Close(root);
}

void cmd_mkdir(CHAR16* input) {
    CHAR16* name = input + 6;
    if (strlen16(name) == 0) { println(L"Usage: mkdir [name]"); return; }
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* cwd_dir = NULL;
    EFI_STATUS s = root->Open(root, &cwd_dir, cwd, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (s != EFI_SUCCESS) {
        println(L"Error: cannot open current directory");
        root->Close(root); return;
    }
    EFI_FILE_PROTOCOL* dir = NULL;
    s = cwd_dir->Open(cwd_dir, &dir, name,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
        EFI_FILE_DIRECTORY);
    if (s == EFI_SUCCESS) { println(L"Directory created."); dir->Close(dir); }
    else {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: could not create directory");
        set_color(EFI_WHITE, EFI_BGBLACK);
    }
    cwd_dir->Close(cwd_dir); root->Close(root);
}

void cmd_del(CHAR16* input) {
    CHAR16* name = input + 4;
    if (strlen16(name) == 0) { println(L"Usage: del [filename]"); return; }
    CHAR16 fullpath[256];
    build_path(cwd, name, fullpath);
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* file = NULL;
    EFI_STATUS s = root->Open(root, &file, fullpath, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: file not found");
        set_color(EFI_WHITE, EFI_BGBLACK); root->Close(root); return;
    }
    s = file->Delete(file);
    if (s == EFI_SUCCESS) { println(L"File deleted."); }
    else {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: could not delete file");
        set_color(EFI_WHITE, EFI_BGBLACK);
    }
    root->Close(root);
}

void cmd_ren(CHAR16* input) {
    CHAR16* p = input + 4;
    CHAR16 oldname[128] = {0};
    int i = 0;
    while (*p && *p != L' ') oldname[i++] = *p++;
    if (*p == L' ') p++;
    if (i == 0 || strlen16(p) == 0) { println(L"Usage: ren [oldname] [newname]"); return; }
    CHAR16 fullpath[256];
    build_path(cwd, oldname, fullpath);
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* file = NULL;
    EFI_STATUS s = root->Open(root, &file, fullpath, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: file not found");
        set_color(EFI_WHITE, EFI_BGBLACK); root->Close(root); return;
    }
    UINT8 infobuf[512];
    UINTN infosize = sizeof(infobuf);
    EFI_GUID finfo_guid = EFI_FILE_INFO_GUID;
    s = file->GetInfo(file, &finfo_guid, &infosize, infobuf);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: cannot get file info");
        set_color(EFI_WHITE, EFI_BGBLACK); file->Close(file); root->Close(root); return;
    }
    EFI_FILE_INFO* info = (EFI_FILE_INFO*)infobuf;
    UINTN j = 0;
    while (p[j]) { info->FileName[j] = p[j]; j++; }
    info->FileName[j] = 0;
    info->Size = 56 + (j + 1) * 2;
    s = file->SetInfo(file, &finfo_guid, info->Size, info);
    if (s == EFI_SUCCESS) { println(L"File renamed."); }
    else {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: could not rename file");
        set_color(EFI_WHITE, EFI_BGBLACK);
    }
    file->Close(file); root->Close(root);
}

void cmd_rmdir(CHAR16* input) {
    CHAR16* name = input + 6;
    int recursive = 0;
    if (startswith(name, L"/s ") || startswith(name, L"/s")) {
        recursive = 1;
        name = name + 2;
        if (*name == L' ') name++;
    }
    if (strlen16(name) == 0) { println(L"Usage: rmdir [dir] or rmdir /s [dir]"); return; }
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* dir = NULL;
    EFI_STATUS s = root->Open(root, &dir, name, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: directory not found");
        set_color(EFI_WHITE, EFI_BGBLACK); root->Close(root); return;
    }
    if (recursive) {
        UINT8 buf[512];
        while (1) {
            UINTN size = sizeof(buf);
            s = dir->Read(dir, &size, buf);
            if (s != EFI_SUCCESS || size == 0) break;
            EFI_FILE_INFO* info = (EFI_FILE_INFO*)buf;
            if (streq(info->FileName, L".") || streq(info->FileName, L"..")) continue;
            EFI_FILE_PROTOCOL* child = NULL;
            s = dir->Open(dir, &child, info->FileName, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
            if (s == EFI_SUCCESS) {
                child->Delete(child);
            }
        }
    }
    s = dir->Delete(dir);
    if (s == EFI_SUCCESS) { println(L"Directory deleted."); }
    else {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: could not delete directory");
        set_color(EFI_WHITE, EFI_BGBLACK);
    }
    root->Close(root);
}

void cmd_copy(CHAR16* input) {
    CHAR16* p = input + 5;
    CHAR16 srcname[128] = {0}, dstname[128] = {0};
    int i = 0;
    while (*p && *p != L' ') srcname[i++] = *p++;
    if (*p == L' ') p++;
    i = 0;
    while (*p) dstname[i++] = *p++;
    if (strlen16(srcname) == 0 || strlen16(dstname) == 0) { println(L"Usage: copy [src] [dst]"); return; }
    CHAR16 srcpath[256], dstpath[256];
    build_path(cwd, srcname, srcpath);
    build_path(cwd, dstname, dstpath);
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* src = NULL;
    EFI_STATUS s = root->Open(root, &src, srcpath, EFI_FILE_MODE_READ, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: source file not found");
        set_color(EFI_WHITE, EFI_BGBLACK); root->Close(root); return;
    }
    EFI_FILE_PROTOCOL* dst = NULL;
    s = root->Open(root, &dst, dstpath, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: cannot create destination file");
        set_color(EFI_WHITE, EFI_BGBLACK); src->Close(src); root->Close(root); return;
    }
    UINT8 buf[512];
    while (1) {
        UINTN size = sizeof(buf);
        s = src->Read(src, &size, buf);
        if (s != EFI_SUCCESS || size == 0) break;
        dst->Write(dst, &size, buf);
    }
    src->Close(src); dst->Close(dst); root->Close(root);
    println(L"File copied.");
}

void cmd_move(CHAR16* input) {
    CHAR16* p = input + 5;
    CHAR16 srcname[128] = {0}, dstname[128] = {0};
    int i = 0;
    while (*p && *p != L' ') srcname[i++] = *p++;
    if (*p == L' ') p++;
    i = 0;
    while (*p) dstname[i++] = *p++;
    if (strlen16(srcname) == 0 || strlen16(dstname) == 0) { println(L"Usage: move [src] [dst]"); return; }
    cmd_copy(input);
    CHAR16 delcmd[256] = L"del ";
    UINTN j = 4;
    for (i = 0; srcname[i]; i++) delcmd[j++] = srcname[i];
    delcmd[j] = 0;
    cmd_del(delcmd);
}

void cmd_more(CHAR16* input) {
    CHAR16* name = input + 5;
    if (strlen16(name) == 0) { println(L"Usage: more [filename]"); return; }
    CHAR16 fullpath[256];
    build_path(cwd, name, fullpath);
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* file = NULL;
    EFI_STATUS s = root->Open(root, &file, fullpath, EFI_FILE_MODE_READ, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: file not found");
        set_color(EFI_WHITE, EFI_BGBLACK); root->Close(root); return;
    }
    UINT8 buf[512];
    CHAR16 out[2] = {0, 0};
    int lines = 0;
    while (1) {
        UINTN size = sizeof(buf);
        file->Read(file, &size, buf);
        if (size == 0) break;
        for (UINTN i = 0; i < size; i++) {
            if (buf[i] == '\n') { 
                println(L""); lines++;
                if (lines >= 20) {
                    println(L"Press any key to continue...");
                    EFI_INPUT_KEY key;
                    while (ST->ConIn->ReadKeyStroke(ST->ConIn, &key) != EFI_SUCCESS);
                    lines = 0;
                }
                continue;
            }
            if (buf[i] == '\r') continue;
            out[0] = (CHAR16)buf[i]; print(out);
        }
    }
    println(L""); file->Close(file); root->Close(root);
}

void cmd_tree_recursive(EFI_FILE_PROTOCOL* dir, int depth) {
    UINT8 buf[512];
    CHAR16 indent[32] = {0};
    for (int i = 0; i < depth*2 && i < 31; i++) indent[i] = L' ';
    while (1) {
        UINTN size = sizeof(buf);
        EFI_STATUS s = dir->Read(dir, &size, buf);
        if (s != EFI_SUCCESS || size == 0) break;
        EFI_FILE_INFO* info = (EFI_FILE_INFO*)buf;
        if (streq(info->FileName, L".") || streq(info->FileName, L"..")) continue;
        print(indent);
        if (info->Attribute & EFI_FILE_DIRECTORY) {
            println(info->FileName);
            EFI_FILE_PROTOCOL* child = NULL;
            s = dir->Open(dir, &child, info->FileName, EFI_FILE_MODE_READ, 0);
            if (s == EFI_SUCCESS && depth < 8) {
                cmd_tree_recursive(child, depth + 1);
                child->Close(child);
            }
        } else {
            println(info->FileName);
        }
    }
}

void cmd_tree(CHAR16* input) {
    (void)input;
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* dir = NULL;
    EFI_STATUS s = root->Open(root, &dir, cwd, EFI_FILE_MODE_READ, 0);
    if (s != EFI_SUCCESS) { println(L"Error: cannot open directory"); root->Close(root); return; }
    print(L"Directory tree of "); println(cwd);
    cmd_tree_recursive(dir, 0);
    dir->Close(dir); root->Close(root);
}

void cmd_edit(CHAR16* input) {
    static CHAR16 lines[32][64];
    static UINT8 buf[2048];
    static UINT8 out[2048];
    
    CHAR16* name = input + 4;
    if (*name == L' ') name++;
    CHAR16 fullpath[256] = {0};
    int have_name = (strlen16(name) > 0);
    if (have_name) build_path(cwd, name, fullpath);

    int linecount = 0;

    if (have_name) {
        EFI_FILE_PROTOCOL* root = open_root();
        EFI_FILE_PROTOCOL* file = NULL;
        EFI_STATUS s = root->Open(root, &file, fullpath, EFI_FILE_MODE_READ, 0);
        if (s == EFI_SUCCESS) {
            UINTN size = sizeof(buf);
            file->Read(file, &size, buf);
            int pos = 0;
            for (UINTN i = 0; i < size && linecount < 32; i++) {
                if (buf[i] == '\n' || buf[i] == '\r') {
                    lines[linecount][pos] = 0;
                    linecount++;
                    pos = 0;
                    if (buf[i] == '\r' && i + 1 < size && buf[i+1] == '\n') i++;
                } else if (pos < 63) {
                    lines[linecount][pos++] = (CHAR16)buf[i];
                }
            }
            if (pos > 0 && linecount < 32) { lines[linecount][pos] = 0; linecount++; }
            file->Close(file);
        }
        root->Close(root);
    }

    while (1) {
        clear();
        set_color(EFI_YELLOW, EFI_BGBLACK); println(L"=== edit mode ==="); set_color(EFI_WHITE, EFI_BGBLACK);
        for (int i = 0; i < linecount; i++) {
            print_uint(i + 1); print(L": "); println(lines[i]);
        }
        if (linecount == 0) println(L"(empty)");
        println(L"Commands: a (append), e <n>, d <n>, w (save), q (cancel)");

        CHAR16 cmd[256];
        read_line(cmd, 256);
        if (streq(cmd, L"q")) { println(L"Edit cancelled."); break; }
        if (streq(cmd, L"w")) {
            if (!have_name) {
                println(L"Filename:"); CHAR16 newname[256]; read_line(newname, 256);
                if (strlen16(newname) == 0) { println(L"Save aborted."); continue; }
                build_path(cwd, newname, fullpath);
                have_name = 1;
            }
            EFI_FILE_PROTOCOL* root = open_root();
            EFI_FILE_PROTOCOL* file = NULL;
            EFI_STATUS s = root->Open(root, &file, fullpath, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
            if (s == EFI_SUCCESS) {
                UINTN outsize = 0;
                for (int i = 0; i < linecount; i++) {
                    for (UINTN j = 0; lines[i][j] && outsize < sizeof(out); j++) out[outsize++] = (UINT8)lines[i][j];
                    if (outsize < sizeof(out)) out[outsize++] = '\n';
                }
                file->Write(file, &outsize, out);
                file->Close(file);
                println(L"File saved.");
            } else {
                set_color(EFI_RED, EFI_BGBLACK); println(L"Error: cannot save file"); set_color(EFI_WHITE, EFI_BGBLACK);
            }
            root->Close(root);
            break;
        }

        if (startswith(cmd, L"a")) {
            if (linecount < 32) {
                println(L"Append:"); read_line(lines[linecount++], 64);
            }
            continue;
        }

        if (startswith(cmd, L"e ")) {
            int idx = 0;
            CHAR16* p = cmd + 2;
            while (*p >= L'0' && *p <= L'9') { idx = idx * 10 + (*p - L'0'); p++; }
            if (idx > 0 && idx <= linecount) {
                println(L"Edit line:"); read_line(lines[idx - 1], 64);
            }
            continue;
        }

        if (startswith(cmd, L"d ")) {
            int idx = 0;
            CHAR16* p = cmd + 2;
            while (*p >= L'0' && *p <= L'9') { idx = idx * 10 + (*p - L'0'); p++; }
            if (idx > 0 && idx <= linecount) {
                for (int j = idx - 1; j < linecount - 1; j++) strcpy16(lines[j], lines[j + 1]);
                linecount--; if (linecount < 0) linecount = 0;
            }
            continue;
        }
    }
}
void cmd_write(CHAR16* input) {
    CHAR16* p = input + 6;
    CHAR16 fname[128] = {0};
    int i = 0;
    while (*p && *p != L' ') fname[i++] = *p++;
    if (*p == L' ') p++;
    if (i == 0 || strlen16(p) == 0) { println(L"Usage: write [filename] [text]"); return; }
    CHAR16 fullpath[256];
    build_path(cwd, fname, fullpath);
    EFI_FILE_PROTOCOL* root = open_root();
    EFI_FILE_PROTOCOL* file = NULL;
    EFI_STATUS s = root->Open(root, &file, fullpath,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (s != EFI_SUCCESS) {
        set_color(EFI_RED, EFI_BGBLACK); println(L"Error: cannot create file");
        set_color(EFI_WHITE, EFI_BGBLACK); return;
    }
    UINTN len = strlen16(p);
    UINT8 abuf[256];
    for (UINTN j = 0; j < len && j < 255; j++) abuf[j] = (UINT8)p[j];
    abuf[len] = '\n';
    UINTN wlen = len + 1;
    file->Write(file, &wlen, abuf);
    file->Close(file); root->Close(root);
    set_color(EFI_GREEN, EFI_BGBLACK); println(L"File written.");
    set_color(EFI_WHITE, EFI_BGBLACK);
}
