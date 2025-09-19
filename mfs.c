/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
 * Project:: Basic File System
 *
 * File:: mfs.c
 *
 * Description::
 *	This is the file system interface.
 *	This is the interface needed by the driver to interact with
 *	your filesystem.
 *
 **************************************************************/
#include "mfs.h"
#include "fsInit.h"
#include "fsLow.h"

//*************************************************************************************************
// Helper functions
//*************************************************************************************************
// Returns index of the DE with name parameter in parent
int findNameInDir(DE *parent, char *name)
{
    if (parent == NULL)
    {
        fprintf(stderr, "Parent is null\n");
        return -1;
    }
    if (name == NULL)
    {
        fprintf(stderr, "File name is null\n");
        return -1;
    }
    int numEntries = parent->size / sizeof(DE);
    for (int i = 0; i < numEntries; i++)
    {
        printf("findNameInDir: parent[%d].name:%s\n", i, parent[i].name);
        if (strcmp(parent[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

//*************************************************************************************************
// Checks if the DE in parent is a directory. 1 if true, 0 if false.
int entryIsDir(DE *parent, int deIndex)
{
    if (parent == NULL)
    {
        fprintf(stderr, "Parent is null\n");
    }
    if (parent[deIndex].isDirectory == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//*************************************************************************************************
// Frees a dir only if not cwd, root, or null.
// Return 1 indicates freeing the dir, 0 indicates no memory freed.
int freeIfNotNeedDir(DE *dir)
{
    if (dir != NULL)
    {
        if (dir != cwdGlobal)
        {
            if (dir != rootGlobal)
            {
                free(dir);
                printf("Freeing directory check. dir:%s\n", dir->name);

                return 1;
            }
        }
    }
    else
    {
        return 0;
    }
}

//*************************************************************************************************
// Find first unused DE in a parent DE. Returns -1 if failed, returns index of DE if success.
int findUnusedDE(DE *parent)
{
    if (parent == NULL)
    {
        fprintf(stderr, "Parent is null\n");
        return 0;
    }

    int numDEs = parent->size / sizeof(DE);
    printf("from findUnusedDE - parent->size:%ld sizeof(DE):%ld\n", parent->size, sizeof(DE));
    for (int i = 2; i < numDEs; i++)
    {
        if (parent[i].name[0] == '\0')
        {
            printf("findUnusedDE index:%d\n", i);
            return i;
        }
    }

    return -1;
}

//*************************************************************************************************
// Write an existing directory to disk; return -1 if failed, 1 if success
int saveDir(DE *directory)
{
    if (directory == NULL)
    {
        fprintf(stderr, "Directory is null\n");
        return -1;
    }

    int returnInt = LBAwrite(directory, directory->dirNumBlocks, directory->LBAlocation);
    return 1;
}

//*************************************************************************************************
/*
parsePath loads the parent in a path and finds if the file (last element) exists or not.
When using this, use parsePath(char * path, &ppi)
Return values of this function:
1. (int)Success or error - check if each index before le is a valid directory
^This is the true return value for parsePath. The rest will be in struct ppinfo.
2.  DE * parentPointer to parent loaded in memory
3. char * lastElement is the name of the last element in the path
4. int lastElementIndex - which entry is it inside the parent? -1 if not exist
*/
int parsePath(char *path, ppinfo *ppi)
{
    if (path == NULL)
    {
        fprintf(stderr, "Path is null\n");
        return 1;
    }
    DE *start;

    if (path[0] == '/')
    {
        if (rootGlobal != NULL)
        {
            start = rootGlobal;
            strcpy(cwdName, "/");
        }
        else
        {
            fprintf(stderr, "Root is null\n");
        }
    }
    else
    {
        if (cwdGlobal != NULL)
        {
            start = cwdGlobal;
        }
    }

    DE *parent = start;

    char *saveptr;
    char *token1 = strtok_r(path, "/", &saveptr);
    // Special case: If the only token is /, then it’ll return null
    if (token1 == NULL)
    {
        if (path[0] != '/')
        {
            return -1; // Invalid path
        }
        ppi->parent = parent;
        ppi->le = NULL;
        ppi->lei = 0; // this means path is root
        return -2;    // Unique return val for root path
    }
    char *token2;

    do
    {
        ppi->le = token1;

        ppi->lei = findNameInDir(parent, token1);
        printf("pp debug - ppi->lei:%d\n", ppi->lei);
        token2 = strtok_r(NULL, "/", &saveptr);
        printf("pp debug - token2:%s\n", token2 ? token2 : "NULL");
        printf("pp debug print token1:%s, ppi->le:%s\n", token1, ppi->le);
        // Success: If token2 is null then token1 is the last element.
        if (token2 == NULL)
        {
            ppi->parent = parent;
            printf("pp debug 4: ppi->parent->name:%s\n", ppi->parent->name);
            printf("pp debug2 print token1:%s, ppi->le:%s\n", token1, ppi->le);
            return (0);
        }
        // If token2 is not null, that tells you token1 has to exist and must be a directory.
        if (ppi->lei < 0) // the name doesn’t exist, invalid path
        {
            printf("pp debug 1\n");
            fprintf(stderr, "Invalid path\n");
            return -1;
        }

        // Helper function EntryisDir
        if (entryIsDir(parent, ppi->lei) == 0)
        {
            printf("pp debug 2\n");
            fprintf(stderr, "Invalid path\n");
            return -1;
        }
        // Now we know token 1 does exist, is valid, and is a directory. So we want to load it/get
        // that dir parsePath
        if (ppi->lei >= (parent->size / sizeof(DE)))
        {
            fprintf(stderr, "ppi->lei is out of bounds");
            return -1;
        }
        DE *temp = loadDirDE(&(parent[ppi->lei]));

        // Helper function freeIfNotNeedDir(parent)
        freeIfNotNeedDir(parent); // not null, not cwd, not root
        parent = temp;
        token1 = token2;
        printf("parsepath round complete\n");

    } while (token2 != NULL);
    // if the index is invalid, exit
    // If the index was valid but not a directory, exit.
    // If it was, then valid!
}


//*************************************************************************************************
// End helper functions
//*************************************************************************************************

// Make a directory; return -1 if fails, 2 if directory already exists, 0 if success.
int fs_mkdir(const char *path, mode_t mode)
{
    if (path == NULL)
    {
        fprintf(stderr, "Path is null\n");
        return -1;
    }

    // Create a writable copy of the path
    char *pathCopy = strdup(path);
    if (pathCopy == NULL)
    {
        fprintf(stderr, "Failed to duplicate path\n");
        return -1;
    }
    ppinfo ppi;
    int parseFlag = parsePath(pathCopy, &ppi);
    // free(pathCopy); //this statement alters ppi.le for some reason
    //  If parsePath fails
    if (parseFlag != 0)
    {
        fprintf(stderr, "parsePath failed\n");
        return (parseFlag);
    }

    // If ppi.lei is not -1, then the directory already exists. Return 2.
    if (ppi.lei != -1)
    {
        fprintf(stderr, "Directory already exists\n");
        return (2);
    }

    // DEBUG this just had ppi.parent as second parameter before
    DE *newDir = initDir(MAX_ENTRIES, &(ppi.parent[ppi.lei]), bm);

    if (newDir == NULL)
    {
        fprintf(stderr, "Unable to create newDir\n");
        return -1;
    }

    int x = findUnusedDE(ppi.parent);

    if (x == -1)
    {
        fsRelease(bm, newDir->LBAlocation, newDir->dirNumBlocks);
        free(newDir);
        fprintf(stderr, "No unused DE in parent");
        return -1;
    }
    printf("ppi.parent time creation:%ld\n", ppi.parent->timeCreation);
    memcpy(&(ppi.parent[x]), newDir, sizeof(DE)); // this is supposed to set newDir to ppi.parent[x].
    // then ppi.le is supposed to be the name...but ppi.le might not be correct.
    printf("from fs_mkdir ppi.le:%s", ppi.le);
    strncpy(ppi.parent[x].name, ppi.le, sizeof(ppi.parent[x].name) - 1);
    ppi.parent[x].name[sizeof(ppi.parent[x].name) - 1] = '\0';
    printf("from fs_mkdir ppi.parent[x].name:%s", ppi.parent[x].name);

    int uDRet = updateDELBA(newDir);

    if (x == 1)
    {
        printf("updateDELBA failed from fs_mkdir\n");
        return -1;
    }
    // PICKUP HERE - You last updated saveDir(newDir) to be updateDELBA instead. Having a hard time
    // conceptualizing how to know which directories belong to which from pure disk memory.

    freeIfNotNeedDir(newDir);

    return 0;
}

//*************************************************************************************************
// Open a directory. Returns NULL if fails. This should also load a directory into memory.
fdDir *fs_opendir(const char *pathname)
{
    if (pathname == NULL)
    {
        fprintf(stderr, "Path is null\n");
        return NULL;
    }
    // Create a writable copy of the path
    char *pathCopy = strdup(pathname);
    if (pathCopy == NULL)
    {
        fprintf(stderr, "Failed to duplicate path\n");
        return NULL;
    }
    ppinfo ppi;
    int parseFlag = parsePath(pathCopy, &ppi);
    // free(pathCopy);

    // If the directory wasn't found, return failure.
    if (ppi.lei == -1)
    {
        fprintf(stderr, "Directory does not exist\n");
        return NULL;
    }

    printf("fs_opendir: ppi.lei:%d\n", ppi.lei);
    DE *thisDir = loadDirLBA((ppi.parent[ppi.lei]).dirNumBlocks, (ppi.parent[ppi.lei]).LBAlocation);

    if (thisDir == NULL)
    {
        fprintf(stderr, "File is not a directory\n");
        return NULL;
    }

    // x counts the number of DEs in thisDir
    int cntEntries = thisDir->size / sizeof(DE);
    /*int x = 0;

    while ((thisDir[x].name[0] == '\0') && x < cntEntries)
    {
        ++x;
    }
    if (x < cntEntries)
    {
        */
    fdDir *fdDirIP = malloc(sizeof(fdDir));

    if (fdDirIP == NULL)
    {
        fprintf(stderr, "fdDir malloc failed");
        freeIfNotNeedDir(thisDir);
        return NULL;
    }

    for (uint32_t i = 0; i < sizeof(fdDir); i++)
    {
        ((char *)fdDirIP)[i] = 0;
    }

    fdDirIP->di = malloc(sizeof(struct fs_diriteminfo));
    if (fdDirIP->di == NULL)
    {
        fprintf(stderr, "fdDirIP->di failed");
        free(fdDirIP);
        freeIfNotNeedDir(thisDir);
        return NULL;
    }
    for (uint32_t i = 0; i < sizeof(struct fs_diriteminfo); i++)
    {
        ((char *)fdDirIP->di)[i] = 0;
    }
    // DEBUG open doesn't actually return a name yet
    fdDirIP->di->d_reclen = sizeof(struct fs_diriteminfo);
    fdDirIP->di->fileType = thisDir->isDirectory == 1 ? FT_DIRECTORY : FT_REGFILE;
    strncpy(fdDirIP->di->d_name, thisDir->name, 255);
    fdDirIP->di->d_name[strlen(thisDir->name)] = '\0';
    fdDirIP->directory = thisDir;
    fdDirIP->numEntries = cntEntries;
    fdDirIP->dirEntryPosition = 0;

    return fdDirIP;
    /*}
    else
    {
        return NULL;
    }
    */
}
//*************************************************************************************************
struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    if (dirp == NULL)
    {
        fprintf(stderr, "fDirectory is invalid");
        return NULL;
    }

    if (dirp->dirEntryPosition >= dirp->numEntries)
    {
        return NULL;
    }

    // Skip past any null DEs in the directory
    while ((dirp->directory[dirp->dirEntryPosition]).name[0] == '\0' &&
           dirp->dirEntryPosition < dirp->numEntries)
    {
        dirp->dirEntryPosition++;
    }

    if (dirp->dirEntryPosition >= dirp->numEntries)
    {
        printf("fs_readdir: No more filled entries in dirp, return\n");
        return NULL;
    }

    DE *newDE = &(dirp->directory[dirp->dirEntryPosition]);
    // printf("from fs_readdir newDE->name:%s\n", newDE->name);
    strncpy(dirp->di->d_name, newDE->name, sizeof(newDE->name));
    // printf("from fs_readdir dirp->di->d_name:%s\n", dirp->di->d_name);
    dirp->di->fileType = newDE->isDirectory == 1 ? FT_DIRECTORY : FT_REGFILE;
    dirp->dirEntryPosition++;

    return dirp->di;
}
//*************************************************************************************************
// closedir frees the resources from opendir
int fs_closedir(fdDir *dirp)
{
    if (dirp == NULL)
    {
        fprintf(stderr, "Directory doesn't exist");
        return 0;
    }
    freeIfNotNeedDir(dirp->directory);
    free(dirp->di);
    free(dirp);
    return 1;
}
//*************************************************************************************************
// Return 0 if success, -1 if fail
int fs_setcwd(char *pathname)
{
    ppinfo ppi;
    int parseFlag = parsePath(pathname, &ppi);

    // If parsePath fails, return error
    if (parseFlag == -1)
    {
        fprintf(stderr, "Invalid pathname");
        return (parseFlag);
    }
    // If parsePath resolves to root
    if (ppi.lei == -2)
    {
        cwdGlobal = rootGlobal;
        return 0;
    }

    if (fs_isFile(pathname) == 1)
    {
        fprintf(stderr, "Path is not a directory");
        return -1;
    }

    // If the path is valid, update the current working directory.
    if (fs_isDir(pathname) == 1)
    {
        cwdGlobal = &(ppi.parent[ppi.lei]);
        strcpy(cwdName, pathname);
        cwdName[strlen(cwdName)] = '\0';
        return 0;
    }
}
//*************************************************************************************************
// Returns 1 if success, 0 if fail
int isNullTerminated(char *str, size_t len)
{
    if (str == NULL)
        return 0; // Check for null pointer

    for (int i = 0; i < len; ++i)
    {
        if (str[i] == '\0')
        {
            return 1;
        }
    }
    return 0;
}

//*************************************************************************************************
// Copies the cwd into the user's buffer pathname. Return the pathname if success, NULL if error.
char *fs_getcwd(char *pathname, size_t size)
{
    if (pathname == NULL)
    {
        fprintf(stderr, "Null pathname");
        return NULL;
    }

    if (size <= strlen(cwdName))
    {
        fprintf(stderr, "Buffer size is too small");
        return NULL;
    }
    if (isNullTerminated(cwdName, size) == 1)
    {
        strncpy(pathname, cwdName, size - 1);
        pathname[size - 1] = '\0'; // Ensure null termination
        return pathname;
    }
    else
    {
        fprintf(stderr, "Source string is not null-terminated");
        return NULL;
    }
}

//*************************************************************************************************
// Return 1 if file, 0 if not
int fs_isFile(char *filename)
{
    if (filename == NULL)
    {
        fprintf(stderr, "Null filename");
        return 0;
    }
    ppinfo ppi;
    parsePath(filename, &ppi);
    if (ppi.parent[ppi.lei].isDirectory == 0 && ppi.parent[ppi.lei].name[0] != '\0')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//*************************************************************************************************
// Returns 1 if path is a directory, 0 if fail.
int fs_isDir(char *pathname)
{

    if (pathname == NULL)
    {
        fprintf(stderr, "Null pathname");
        return 0;
    }
    ppinfo ppi;
    parsePath(pathname, &ppi);
    if (ppi.parent[ppi.lei].isDirectory == 1 && ppi.parent[ppi.lei].name[0] != '\0')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//*************************************************************************************************
// Returns 0 if success, -1 if failure
int fs_stat(const char *path, struct fs_stat *buf)
{
    if (path == NULL)
    {
        fprintf(stderr, "Path is null\n");
        return -1;
    }

    // Create a writable copy of the path
    char *pathCopy = strdup(path);
    if (pathCopy == NULL)
    {
        fprintf(stderr, "Failed to duplicate path\n");
        return -1;
    }
    ppinfo ppi;
    int parseFlag = parsePath(pathCopy, &ppi);
    free(pathCopy);

    DE *de = &(ppi.parent[ppi.lei]);
    buf->st_size = de->size;
    buf->st_blksize = vcb->block_size;
    buf->st_blocks = vcb->total_blocks;
    buf->st_accesstime = de->lastAccessed;
    buf->st_modtime = de->lastModified;
    buf->st_createtime = de->timeCreation;
    return 0;
}
