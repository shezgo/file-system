#include "mfs.h"
#include "fsInit.c"
#include "fsInit.h"
#include "fsLow.h"


//*****************************************
// Helper functions

// Returns index of the DE with name parameter in parent
int findNameInDir(DE *parent, char *name) 
{
    if (parent == NULL)
    {
        fprintf(stderr, "Parent is null");
        return -1;
    }

    int numEntries = parent->size / sizeof(DE);

    for (int i = 0; i < numEntries; i++)
    {
        if (strcmp(parent[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}
// Loads a directory into memory for manipulation
DE * loadDir(DE *dir)
{
   if(dir->isDirectory == 0)
   {
    fprintf(stderr, "DE is not a directory.");
    return;
   }

   DE *loadedDir = malloc(dir->dirNumBlocks * vcb->block_size);
   LBAread(loadedDir, dir->dirNumBlocks, dir->LBAlocation);
   return loadedDir;
}

// Checks if the DE in parent is a directory. 1 if true, 0 if false.
int entryIsDir(DE *parent, int deIndex)
{
    if(parent == NULL)
    {
        fprintf(stderr, "Parent is null");
    }
    if(parent[deIndex].isDirectory == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//Frees a dir only if not cwd, root, or null.
//Return 1 indicates freeing the dir, 0 indicates no memory freed.
int freeIfNotNeedDir(DE *dir) 
{
    if(dir != NULL)
    {
        if(dir != cwdGlobal)
        {
            if(dir != rootGlobal)
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

/*parsePath loads the parent in a path and finds if the file (last element) exists or not.
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
    }
    DE *start;
    if (path[0] == '/')
    {
        if (rootGlobal != NULL)
        {
            start = rootGlobal; // what am i assuming here? That rootDir is a global variable - already //loaded into memory
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
            // Keep CurrWorkDir and rootDir in Ram. When switch dirs, switch the currentworkingdir so they //never get switched. Root is loaded forever.
        }
    }

    DE *parent = start;
    ppinfo *ppi;

    // saveptr is required by strtok_r
    char *saveptr;
    char *token1 = strtok_r(path, "/", &saveptr);
    // Special case: what if only token in it is /? Then it’ll return null
    if (token1 == NULL)
    {
        ppi->parent = parent;
        ppi->le = NULL;
        ppi->lei = -2; // it’s not that it doesn’t exist, there’s no last element. Sentinel value for / only
        return (0);
    }

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
            return;
        }

        // Helper function EntryisDir
        if (entryIsDir(parent, ppi->lei) == 0)
        {
            fprintf(stderr, "Invalid path");
            return;
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

} // if the index is invalid, get out of here. If the index was valid but not a directory, get out of here. If it was, then valid!
//End helper functions
//***********************************

int mkdir(char *path, mode_t mode)
{
    ppinfo ppi;
    int ret = parsePath(path, &ppi);
    if (ret != 0)
    {
        return (ret);
    } // parsePath failed, so we know the path is meaningless. Get out.
    // now we know parse path succeeded, so we go…
    if (ppi.lei != -1)
    {
        return (2);
    } // the 2 represents some error message but it shouldnt be 2 says lecture
    // Cause it knows hey, it already exists, I don’t wanna be here.
    // Now we say we know to make a directory, so let’s do it.
    DE *newDir = initDir(DEFAULT_CNT, ppi.parent); // ppi.parent and &(ppi.parent[0]) are identical

    // Now find the index of an unused DE in the parent - make helper function
    int x = findUnusedDE(ppi.parent);
    if (x == -1)
    {
        // this is an error case
        Free Block new dir // remember initDir allocates a whole block, fsRelease?
            Free new dir   // free(newDir); //probably
                Return error
    }
    // if it’s not -1. The newDir is the dot entry of newDir (index 0)
    memcpy(&(ppi.parent[x]), newDir, sizeof(DE));
    strcpy(ppi.parent[x].name, ppi.le);
    // Now we need to write/save this directory. Also need to do that in initDir function, let’s make a //helper function
    // This savedir will write the directory to disk. EZ, directories are a whole block
    savedir(ppi.parent)
        Free(newdir)

        // Helper function:if it’s the root dir or cwd do not free it; else yes
        Free
        ifnotneededDir(ppi.parent)
}

//******************************************
fdDir *fs_opendir(const char *pathname)
{
    DE *thisdir = magic(“pathname”) // Now a directory that was on disk is loaded into RAM
                                    // In RAM, this looks like an array of DEs. The first one is ., the second one is ..
                                    // Let’s start from 0. Then we cann readdir and pass in this structure. Im gonna look at this pointer, sub? which one I’m on.

                  // how do we find out how many entries there are?
—->Cntentries = ptr[0].size / sizeof(DE)
                                   While ptr[x] is not used(an unused entry),
       &&x < CNTEntries
    {
        ++x
                If x < CNTentries
        {
            // Then I have something to give them
            // the fdDir structture
            fdDir infoPointer.di.reclen = sizeof(*di)
                                              fdDirIP.di.fileType = ptr[x].isDir ? FT_DIR : FT_REG // if true set to FT_DIR elseREG
                                                                                                strncpy(fdDirIP.di.name, ptr[x].name, 255) fdDirIP.currententry = x + 1;
            Return di;
        }
        Else { return NULL }
    }
}