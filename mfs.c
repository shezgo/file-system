#include "mfs.h"
#include "fsInit.h"
#include "fsLow.h"

//*****************************************
// Helper functions

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
        printf("parent[i].name:%s\n", parent[i].name);
        if (strcmp(parent[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}
// Loads a directory into memory for manipulation
DE *loadDir(DE *dir)
{
    if (dir->isDirectory == 0)
    {
        fprintf(stderr, "loadDir: DE is not a directory.\n");
        return NULL;
    }
    if(strcmp(dir->name, rootGlobal->name)== 0){

        return rootGlobal;
    }

    DE *loadedDir = malloc(dir->dirNumBlocks * vcb->block_size);
    LBAread(loadedDir, dir->dirNumBlocks, dir->LBAlocation);
    return loadedDir;
}

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
                return 1;
            }
        }
    }
    else
    {
        return 0;
    }
}

// Find first unused DE in a parent DE. Returns -1 if failed, returns index of DE if success.
int findUnusedDE(DE *parent)
{
    if (parent == NULL)
    {
        fprintf(stderr, "Parent is null\n");
        return 0;
    }

    /*
        Pseudocode to find first DE in parent
        parent[i] is a DE. Check if the DE.name[0] is set to '\0' If so, then unused.
        If you traverse the whole thing, then there is no unused DE. DE is full.

        How should I track number of DEs in the parent?
        number of DEs is parent.size/sizeof(DE)
    */
    int numDEs = parent->size / sizeof(DE);
    for (int i = 2; i < numDEs; i++)
    {
        if (parent[i].name[0] == '\0')
        {
            return i;
        }
    }

    return -1;
}

// Write an existing directory to disk; return -1 if failed, 1 if success
int saveDir(DE *directory)
{
    if (directory == NULL)
    {
        fprintf(stderr, "Directory is null\n");
        return -1;
    }

    LBAwrite(directory, directory->dirNumBlocks, directory->LBAlocation);
    return 1;
}

/*parsePath loads the parent in a path and finds if the file (last element) exists or not.
Returns 0 if successful, -1 if not successful.
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
    /*
    //Need to know where to start, is it an absolute or relative path?
    /home/student/documents/foo/bar is an absolute path.
    If I put Documents/foo/bar that’s relative.
    What’s the difference?
    Absolute path starts from root
    What’s the indication between absolute or relative path? If the FIRST character is a slash.
    */

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
            start = cwdGlobal; // also global already loaded.
            // Keep cwd and rootDir in Ram. When switch dirs, switch the cwd so they
            // never get switched. Root is loaded forever.
        }
    }

    DE *parent = start;

    // saveptr is required by strtok_r
    char *saveptr;
    char *token1 = strtok_r(path, "/", &saveptr);
    // Special case: what if only token in it is /? Then it’ll return null
    if (token1 == NULL)
    {
        ppi->parent = parent;
        ppi->le = NULL;
        ppi->lei = 0; // this means path is root
        return -2;
    }

    // Start building the absolute path here and hold in ppi?

    char *token2;

    do
    {
        ppi->le = token1;
        // Need helper function findNameInDir - check if token1 (parent) exists
        // This helper function, given the parent and the name (token1), will return an index.
        // Might be -1 (not found) or whatever index.
        ppi->lei = findNameInDir(parent, token1);
        char *saveptr2;
        token2 = strtok_r(NULL, "/", &saveptr2);
        // If token2 is null then token1 is the last element.
        if (token2 == NULL)
        {
            // We've already initialized all other ppi fields, complete parent.
            ppi->parent = parent;
            return (0);
        }
        // If token2 is not null, that tells you token1 has to exist and must be a directory.
        if (ppi->lei < 0) // the name doesn’t exist, invalid path
        {
            fprintf(stderr, "Invalid path");
            return -1;
        }

        // Helper function EntryisDir
        if (entryIsDir(parent, ppi->lei) == 0)
        {
            fprintf(stderr, "Invalid path");
            return -1;
        }
        // Now we know token 1 does exist, is valid, and is a directory. So we want to load it/get
        // that dir
        // Helper function loadDir
        DE *temp = loadDir(&(parent[ppi->lei]));
        // Helper function freeIfNotNeedDir(parent)
        freeIfNotNeedDir(parent); // not null, not cwd, not root
        parent = temp;
        token1 = token2;
    } while (token2 != NULL);
    // if the index is invalid, get out of here. If the index was valid but not a directory, get out of here.
    // If it was, then valid!
}
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
    free(pathCopy);

    // If parsePath fails
    if (parseFlag != 0)
    {
        return (parseFlag);
    }

    // If ppi.lei is not -1, then the directory already exists. Return 2.
    if (ppi.lei != -1)
    {
        return (2);
    }

    // Now we know to make a directory.
    // ppi.parent and &(ppi.parent[0]) are identical
    DE *newDir = initDir(MIN_ENTRIES, ppi.parent, vcb, bitmap);

    // Now find the index of an unused DE in the parent - 0 if failed, index of DE if success
    int x = findUnusedDE(ppi.parent);

    if (x == -1)
    {
        fsRelease(bitmap, newDir->LBAlocation, newDir->dirNumBlocks);
        free(newDir);
        fprintf(stderr, "No unused DE in parent");
        return -1;
    }
    // if it’s not -1, the newDir is the dot entry of newDir (index 0)
    // Assign the newDir to the unused DE in the parent
    memcpy(&(ppi.parent[x]), newDir, sizeof(DE));
    strcpy(ppi.parent[x].name, ppi.le);
    // Now we need to write/save this directory. Also need to do that in initDir function, let’s make a //helper function
    // This savedir will write the directory to disk. EZ, directories have their blocks tracked.
    // Currently initDir saves a dir to disk. Take that, create a helper function, and use that function both
    // here and there.
    saveDir(newDir);
    free(newDir);
    // Helper function:if it’s NULL, the root dir or cwd do not free it; else yes
    freeIfNotNeedDir(ppi.parent);
}

//************************************************************************
// Open a directory. Returns NULL if fails.
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
    free(pathCopy);

    // If the directory wasn't found, return failure.
    if (ppi.lei == -1)
    {
        fprintf(stderr, "Directory does not exist\n");
        return NULL;
    }

    // Check that path is a directory and load it into memory if so using loadDir
            printf("opendir debug 1\n");
    DE *thisDir = loadDir(&ppi.parent[ppi.lei]);
    if (thisDir == NULL)
    {
        printf("opendir debug 2\n");
        fprintf(stderr, "File is not a directory\n");
        return NULL;
    }
    // In RAM, this looks like an array of DEs. The first one is ., the second one is ..

    //**** NEEDS READDIR HERE? NO! This is how you get the fdDir struct which gets passed into readdir.
    //"readdir is an iterator function"
    // Let’s start from 0. Then we cann readdir and pass in thisdir.
    // "Im gonna look at this pointer, sub? which one I’m on."

    // how do we find out how many entries there are?
    int cntEntries = thisDir->size / sizeof(DE);
    // while ppi.parent[x] is not used(an unused entry) &&x < CNTEntries
    int x = 0;

    while ((thisDir[x].name[0] == '\0') && x < cntEntries)
    {
        ++x;
    }
    if (x < cntEntries)
    {
        fdDir *fdDirIP = malloc(sizeof(fdDir));

        if (fdDirIP == NULL)
        {
            fprintf(stderr, "fdDir malloc failed");
            return NULL;
        }
        fdDirIP->di = malloc(sizeof(struct fs_diriteminfo));
        if (fdDirIP->di == NULL)
        {
            fprintf(stderr, "fdDirIP->di failed");
            free(fdDirIP);
            return NULL;
        }
        // Then I have something to give them
        // "the fdDir structure info pointer" fdDir confirmed
        fdDirIP->di->d_reclen = sizeof(struct fs_diriteminfo);
        fdDirIP->di->fileType = thisDir[x].isDirectory == 1 ? FT_DIRECTORY : FT_REGFILE; // if true set to FT_DIR elseREG
        strncpy(fdDirIP->di->d_name, thisDir[x].name, 255);
        thisDir->name[strlen(thisDir[x].name)] = '\0';
        fdDirIP->directory = &(thisDir[x]);
        fdDirIP->dirEntryPosition = x + 1;
        return fdDirIP;
    }
    else
    {
        return NULL;
    }
}
//**************************************************************************
// reclen? length of the struct itself - fs_diriteminfo. This is conventional for structs
// meaningless for us though
// filetype is fs_diriteminfo is either regfile or directory #defines
// dont exceed name field, use strncpy and make sure it's null terminated

/*
fdDir struct - same reclen,
add what I want
have a copy of the struct that you'll return cause they don't free that - I'm just gonna
keep overwriting it and giving it to them. fs_diritemInfo *di

Why is DE * directory commented out? Cause don't you want this directory loaded into
ram so you can go array[0] [1] [2]?

dirEntryPosition - when open is called, what do you start at? 0. You'll iterate using this for
readdir.

call opendir to initialize the structure.
*/

// every time readdir is called, it gives you the next name. If it's iterated through all the DEs,
// just keep returning null.
struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    /*
        Pseudocode:
        -check dirp for NULL
        -Using an fdDir object (dirp), return a fs_diriteminfo object to the caller.
        -dirp will have an entry position, and a pointer to the directory we'd need to iterate
        through?
        -We're returning a di here. Do any updates need to be made to di? Yes.
        Need to increment dirEntryPosition first every time it's called, which will update
        which DE you grab from directory. How does this effect di?
        di will need to keep getting updated depending on the DE. Update all 3 fields.

    */
    if (dirp == NULL)
    {
        fprintf(stderr, "fDirectory is invalid");
        return NULL;
    }
    /*
        Hang on, we need some clarity here. What is readdir going to do?
        So, we have an open directory given to us with an fdDir (basically a file descriptor)

        We want to parse that and return the next directory entry within that directory.
        This means when we get it, there's going to be what was read the last time it was called.
        The first thing we need to do is pull up the next directory.
        How do we do that? It needs to be the next directory in the parent. Is fdDir going
        to hold the parent already? Yes, directory IS the parent and dirEntryPosition holds
        the current index within that parent.

        So, now dirp is pointing at the data of the next DE. Any other updates needed on dirp?
        Yes, to di.
    */

    DE *newDE = &(dirp->directory[dirp->dirEntryPosition]);
    strncpy(dirp->di->d_name, newDE->name, 255);
    dirp->di->d_name[strlen(dirp->di->d_name)] = '\0';
    dirp->di->fileType = newDE->isDirectory == 1 ? FT_DIRECTORY : FT_REGFILE;
    dirp->dirEntryPosition++;
    return dirp->di;
}
//********************************************************************
// closedir frees the resources from opendir
int fs_closedir(fdDir *dirp)
{
    if (dirp == NULL)
    {
        fprintf(stderr, "Directory doesn't exist");
        return 0;
    }
    free(dirp->directory);
    free(dirp->di);
    free(dirp);
    return 1;
}

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

int fs_stat(const char *path, struct fs_stat *buf)
{
    return 0;
}