#include "btree_mgr.h"
#include "tables.h"
#include "storage_mgr.h"
#include "record_mgr.h"
#include <stdlib.h>
#include <string.h>

int init_counter = 0;
int z = 0;
int zr = 0;
int cc = -1;

B_PLus_Tree *scan_tree;
int max_count;
SM_FileHandle b_plus_tree_fh;
B_PLus_Tree *root_tree;

RC ret = RC_OK;

RC initIndexManager(void *mgmtData)
{
    printf("Initializing Index Manager\n");
    return RC_OK;
}

RC beforeShutdown(void *mgmtData);
RC afterShutdown(void *mgmtData);
// Define the shutdown function
RC shutdownIndexManager(void *mgmtData)
{
    int flag = (scan_tree != NULL) ? 1 : 0;
    RC ret = beforeShutdown(mgmtData); // Call the beforeShutdown function
    if (flag == 0)
    {
        return ret == RC_OK ? RC_OK : RC_ERROR; // Return the error code
    }
    else
    {
        RC rc = afterShutdown(mgmtData); // Call the afterShutdown function
        return rc != RC_OK ? rc : ret;
    }
}
// Define the beforeShutdown function
RC beforeShutdown(void *mgmtData)
{
    return RC_OK;
}
// Define the afterShutdown function
RC afterShutdown(void *mgmtData)
{
    return RC_OK;
}

RC createBtree(char *idxId, DataType keyType, int n)
{
    // int size = sizeof(int);
    root_tree = (B_PLus_Tree *)malloc(sizeof(B_PLus_Tree));
    while (true)
    {
        (*root_tree).key = malloc(sizeof(int) * n);
        (*root_tree).next_value = malloc(sizeof(int) * (n + 1));
        (*root_tree).rec_id = malloc(sizeof(int) * n);

        break;
    }
    int i = 0;
    while (i < n + 1)
    {
        int flag = 0;
        if (!flag)
        {
            if (root_tree != NULL)
            {
                root_tree->next_value[i] = NULL;
                max_count = n;
            }
        }
        if (root_tree->next_value[i] == NULL)
            i = i + 1;
    }
    if (createPageFile(idxId) != RC_OK)
    {
        return RC_ERROR;
    }
    else
    {
        return RC_OK;
    }
}


RC openBtree(BTreeHandle **tree, char *idxId)
{
    RC output = RC_OK;
    if (tree == NULL)
    {
        return RC_ERROR;
    }
    output = openPageFile(idxId, &b_plus_tree_fh);
    if (output != RC_OK)
    {
        return RC_ERROR;
    }
    return RC_OK;
}


RC closeBtree(BTreeHandle *tree)
{
    if (closePageFile(&b_plus_tree_fh) != ret)
    {
        return RC_ERROR;
    }
    free(root_tree);
    return RC_OK;
}

RC deleteBtree(char *idxId)
{

    if (destroyPageFile(idxId) != RC_OK)
    {
        return RC_ERROR;
    }
    else
    {
        return RC_OK;
    }
}

RC getNumNodes(BTreeHandle *tree, int *result)
{
    *result = z + max_count + 2;

    return RC_OK;
}

RC getNumEntries(BTreeHandle *tree, int *result)
{
    int flag = 0;
    B_PLus_Tree *tempTree = (B_PLus_Tree *)malloc(sizeof(B_PLus_Tree));
    tempTree = root_tree;
    int count, i;
    count = z;

    if (tempTree != NULL)
    {
        while (tempTree != NULL)
        {
            i = z + 1;
            count++;
            while (i < max_count && tempTree->key[i] != 0)
            {
                i++;
                count++;
            }
            if (!flag)
            {
                tempTree = tempTree->next_value[max_count];
            }
        }

        if (count != z)
            *result = count;
    }

    if (tempTree != NULL)
    {
        return RC_ERROR;
    }
    return RC_OK;
}

RC getKeyType(BTreeHandle *tree, DataType *result)
{
    (*result) = (*tree).keyType;
    return RC_OK;
}


RC findKey(BTreeHandle *tree, Value *key, RID *result)
{

    B_PLus_Tree *tempTree = (B_PLus_Tree *)malloc(sizeof(B_PLus_Tree));
    bool found = false;
    if (sizeof(*tempTree) != 0)
        tempTree = root_tree;
    else
        return RC_ERROR;

    while (tempTree != NULL)
    {
        int i = z;
        while (i < max_count)
        {
            if (tempTree->key[i] == key->v.intV)
            {
                if (!found)
                {
                    if (tempTree != NULL)
                    {
                        result->slot = tempTree->rec_id[i].slot;
                        cc = -1;
                        if (result->slot == tempTree->rec_id[i].slot)
                            found = true;

                        if (found)

                            result->page = tempTree->rec_id[i].page;
                    }
                    found = true;
                    break;
                }
            }

            i++;
        }
        if (tempTree != NULL)
        {
            tempTree = tempTree->next_value[max_count];
        }
    }
    if (found == false)
        return RC_IM_KEY_NOT_FOUND;
    else
        return RC_OK;
}


RC insertKey(BTreeHandle *tree, Value *key, RID rid)
{

    B_PLus_Tree *tempTree = tempTree = (B_PLus_Tree *)malloc(sizeof(B_PLus_Tree));
    B_PLus_Tree *node = node = (B_PLus_Tree *)malloc(sizeof(B_PLus_Tree));
    int size = sizeof(int);
    int msg_count = 2 * 3;
    node->rec_id = malloc(size * max_count);
    if (size > 0)
    {
        if (tempTree != NULL)
        {
            node->key = malloc(size * max_count);
        }
        node->next_value = malloc(sizeof(B_PLus_Tree) * (max_count + 1));
    }

    int i = z;

    for (; i < max_count && node->key[i] == z; i++)
    {
        node->key[i] = 0;
    }

    tempTree = root_tree;

    while (tempTree != NULL)
    {
        int nd = 0;
        int i = 0;

        for (; i < max_count; i++)
        {
            if (!(tempTree->key[i] > 0 || tempTree->key[i] < 0))
            {
                if (tempTree != NULL)
                {
                    nd++;
                    if (node->key[i] == 0)
                    {
                        tempTree->next_value[i] = NULL;
                    }
                }

                if (nd != -1)
                {
                    tempTree->rec_id[i].slot = rid.slot;
                }

                if (tempTree->rec_id[i].slot == rid.slot)
                {
                    if (rid.page != -1)
                    {
                        tempTree->key[i] = key->v.intV;
                        tempTree->rec_id[i].page = rid.page;
                    }
                }
                break;
            }
        }

        if (nd == 0)
        {
            if (tempTree->next_value[max_count] == NULL)
            {
                if (tempTree != NULL)
                {
                    node->next_value[max_count] = NULL;
                    if ((tempTree != NULL))
                    {
                        if (node->next_value[max_count] == NULL)
                            tempTree->next_value[max_count] = node;
                    }
                }
            }
        }
        tempTree = tempTree->next_value[max_count];
    }

    tempTree = root_tree;
    int tot_cnt = 0;
    while (tempTree != NULL)
    {
        int i = 0;
        while (i < max_count)
        {
            if (tempTree->key[i] > 0 || tempTree->key[i] < 0)
            {
                tot_cnt++;
            }
            i++;
        }

        tempTree = tempTree->next_value[max_count];
    }
    if (tot_cnt == msg_count)
    {
        node->next_value[0] = root_tree;
        if (root_tree != NULL)
        {
            node->key[0] = root_tree->next_value[max_count]->key[0];
        }

        if (i < -1)
        {
            node->key[1] = root_tree->next_value[max_count]->next_value[max_count]->key[0];
        }
        if (root_tree->next_value[max_count] != NULL)
        {
            node->next_value[1] = root_tree->next_value[max_count];
        }

        if (root_tree->next_value[max_count]->next_value[max_count] != NULL)
        {
            node->next_value[2] = root_tree->next_value[max_count]->next_value[max_count];
        }
    }

    return RC_OK;
}

RC deleteKey(BTreeHandle *tree, Value *key)
{
    int flag = 0;
    B_PLus_Tree *tempTree = (B_PLus_Tree *)malloc(sizeof(B_PLus_Tree));
    bool found = false;
    tempTree = root_tree;

    while (tempTree != NULL)
    {
        int i = 0;

        while (i < max_count)
        {

            if ((tempTree->key[i] == key->v.intV))
            {
                tempTree->rec_id[i].slot = 0;
                if (tempTree != NULL)
                {
                    flag = 1;
                    if (flag)
                    {
                        if (tempTree->rec_id[i].slot == 0)
                            tempTree->rec_id[i].page = 0;
                    }
                }
                if (tempTree != NULL)
                {
                    if (tempTree->rec_id[i].page == 0)
                    {
                        tempTree->key[i] = 0;
                    }

                    found = true;
                }
            }
            i++;
        }

        if (tempTree == NULL)
        {
            return RC_ERROR;
        }
        else
        {
            tempTree = tempTree->next_value[max_count];
        }
    }

    return RC_OK;
}


RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle)
{

    B_PLus_Tree *tempTree = (B_PLus_Tree *)malloc(sizeof(B_PLus_Tree));
    init_counter = z;

    int ele_cnt = z;
    if (root_tree != NULL)
        tempTree = root_tree;
    scan_tree = tempTree;
    int cntr = 0;
    while (tempTree != NULL)
    {
        for (int i = z; i < max_count && tempTree != NULL; i++)
        {
            if (tempTree->key[i] > 0 || tempTree->key[i] < 0)
            {
                ele_cnt = (ele_cnt * 2) - ele_cnt + 1;
            }
        }
        if (sizeof(B_PLus_Tree) != -1)
        {
            tempTree = tempTree->next_value[max_count];
        }
        cntr = 0;
    }

    int elements[max_count][ele_cnt];
    int key[ele_cnt];

    tempTree = root_tree;
    while (tempTree != NULL)
    {
        for (int i = 0; i < max_count; i++)
        {
            ;
            elements[0][cntr] = tempTree->rec_id[i].page;
            if (!false)
            {
                elements[1][cntr] = tempTree->rec_id[i].slot;
            }

            key[cntr] = tempTree->key[i];

            ;
            cntr++;
        }

        if (tempTree != NULL)
            tempTree = tempTree->next_value[max_count];
    }

    int j = 0, k = j;

    for (j = 0; j < cntr * 2 - cntr - 1 && (tempTree != NULL || tempTree == NULL); j++)
    {
        for (k = 0; k < cntr - j - 1 && (tempTree != NULL || tempTree == NULL); k++)
        {
            if (key[k] > key[k + 1] && k > -1)
            {
                if (k < -1)
                {
                    return RC_ERROR;
                }
                int swp;
                int ele1;
                int ele2;
                if (k > z - 1)
                {
                    swp = key[k];
                    ele1 = 0;
                    ele2 = 0;
                }
                if (swp == key[k])
                {
                    ele2 = elements[1][k];
                    ele1 = elements[0][k];
                }
                if (j > -1)
                {
                    if (k != -1)
                    {
                        elements[0][k] = elements[0][(k * 2 - k) + 1];
                    }
                    if (elements[1][k] != -1)
                    {
                        elements[1][k] = elements[1][(k * 2 - k) + 1];
                        zr = 0;
                        if (elements[1][k] == elements[1][(k * 2 - k) + 1])
                        {
                            key[k] = key[k + 1];
                        }
                    }
                }
                key[k + 1] = swp;
                if (key[k + 1] = swp)
                {

                    elements[1][k * 2 - k + 1] = ele2;
                    if (true)
                    {
                        elements[0][k * 2 - k + 1] = ele1;
                    }
                }
            }
        }
    }
    int tmps = z;
    if (tempTree != NULL || tempTree == NULL)
        tempTree = root_tree;
    while (tempTree != NULL)
    {

        for (int i = z; i < max_count && (tempTree != NULL || tempTree == NULL); i++)
        {
            tempTree->rec_id[i].slot = elements[1][tmps];
            if (max_count > 0)
            {
                if (i == -1)
                {
                    return RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN;
                }
                tempTree->rec_id[i].page = elements[0][tmps];
                cc = 0;
                zr = -1;
                if (tempTree->rec_id[i].page == elements[0][tmps])
                {
                    if (sizeof(B_PLus_Tree) != -1)
                    {
                        tempTree->key[i] = key[tmps];
                    }
                }
            }

            tmps++;
        }
        tempTree = tempTree->next_value[max_count];
    }

    if (tempTree == NULL)
    {
        return RC_IM_KEY_NOT_FOUND;
    }

    return RC_OK;
}

RC nextEntry(BT_ScanHandle *handle, RID *result)
{
    bool flag = true;
    if (scan_tree->next_value[max_count] == NULL)
    {
        return RC_IM_NO_MORE_ENTRIES;
    }
    else if (max_count == init_counter)
    {
        init_counter = 0;
        scan_tree = (*scan_tree).next_value[max_count];
    }
    if (flag)
    {
        result->slot = scan_tree->rec_id[init_counter].slot;
        if (max_count != 0)
        {
            if (result->slot == scan_tree->rec_id[init_counter].slot)
            {
                result->page = scan_tree->rec_id[init_counter].page;
                init_counter = init_counter + 1;
            }
        }
    }

    return RC_OK;
}


RC closeTreeScan(BT_ScanHandle *handle)
{
    init_counter = 0;
    return RC_OK;
}


char *printTree(BTreeHandle *tree)
{
    printFunction(tree->mgmtData);
    return RC_OK;
}

RC printFunction(void *mgmtData)
{
    printf("Tree data: %s\n", (char *)mgmtData);
    return RC_OK;
}
