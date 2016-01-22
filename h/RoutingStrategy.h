#ifndef _ROUTING_STRATEGY_H_
#define _ROUTING_STRATEGY_H_

#include <functional>
#include <string>
#include <unordered_map>

namespace QB{
	
	
	/* 
	 * @brief Class to control instruments routing to right thread
	 */
	struct SimpleHash
	{
		static size_t getId(std::string& symbol) 
		{
			
			static std::unordered_map<std::string, std::size_t> cache;
			std::unordered_map<std::string, std::size_t>::iterator iter = cache.begin();
			if (iter != cache.end())
			{	// We have hash in cache
				return iter->second;
			}
			
			std::hash<std::string> hash_fn;
			std::size_t str_hash = hash_fn(symbol);
			//cache.insert(std::make_pair(symbol, str_hash));
			return (str_hash % MAX_THREADS) ; 
		}
		
		
	};

}
#endif
 