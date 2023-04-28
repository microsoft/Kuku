#include<vector>
#include"kuku/kuku.h"
#include"kuku/common.h"

namespace kuku
{
    /*
    Each bucket class represents the bucket held at each index of the kuku hash table
    it has two primary operations, insert and get
    */
    class bucket {
        public: 
            bucket(size_t bucket_size) {
                bucketArray = std::vector<item_type>();
                bucketLength = bucketArray.max_size();
            }
            //inserts an item into the bucket, returning a displaced element if it exists, the inserted item otherwise
            item_type insert(item_type item) {
                //insert item to bucket
                if(bucketArray.size() >= bucketLength) {
                    //bucket cannot have anymore items, replace something in the bucket
                    item_type temp = bucketArray[0];
                    bucketArray.erase(bucketArray.begin());
                    bucketArray.emplace_back(item);
                    return temp;
                }
                return bucketArray.emplace_back(item);
            }

            item_type get(int index) {
                return bucketArray[i];
            }
            //Returns index of item if bucket contains item, -1 otherwise
            int contains(item_type item) {
                for(int i = 0; i < bucketLength; ++i) {
                    if(are_equal_item(bucketArray[i], item)) {
                        return i;
                    }
                }
                return -1;
            }

            size_t length() {
                return bucketArray.size();
            }
        private:
            std::vector<item_type> bucketArray;
            size_t bucketLength;
    }
}