#ifndef ORG_EEROS_SAFETY_SAFETYSYSTEM_HPP_
#define ORG_EEROS_SAFETY_SAFETYSYSTEM_HPP_

#include <vector>
#include <eeros/core/Runnable.hpp>
#include <eeros/safety/SafetyLevel.hpp>
#include <eeros/safety/SafetyProperties.hpp>
#include <eeros/safety/SafetyContext.hpp>

namespace eeros {
	namespace safety {
		
		// Forward declarations
		class SystemInputInterface;
		class SystemOutputInterface;
		
		class SafetySystem : public Runnable {
		public:
			SafetyLevel& getCurrentLevel(void);
			SafetyLevel& getLevelById(int32_t levelId);
			SafetyLevel& operator[](unsigned levelId);
			
			void triggerEvent(uint32_t event, SafetyContext* context = nullptr);
			void run();
			bool setProperties(SafetyProperties properties);
			
			static SafetySystem& instance();
			
		private:
			SafetySystem();
			SafetySystem(const SafetySystem&);
			SafetySystem& operator=(const SafetySystem&);
			
			SafetyProperties properties;
			SafetyLevel* currentLevel;
			SafetyContext privateContext;
			
		};

	};
};

#endif // ORG_EEROS_SAFETY_SAFETYSYSTEM_HPP_
