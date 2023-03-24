#pragma once
// This class manages all data to do with actors
//
#include "models/serde.hpp"
#include "data/covsave.hpp"
#include "models/gactor.hpp"
#include "events.hpp"

class Actors: public Serde, public EventListener, public DirectSerde {
  public:
    Actors& GetSingleton() noexcept {
  		static Actors instance;
  		return instance;
  	}

    // Serde methods
    virtual toml::value SaveData() override {
      toml::array array;
      for (const auto& [formID, actorPtr]: this->actors) {
        GActor& actor = actorPtr.get();
        try {
          auto& factory = this->factories.at(actor.GetCls());

          array.push_back(
            toml::value(
              {
                {"formid", actor.GetFormID()},
                {"name", actor.GetName()},
                {"cls", actor.GetCls()},
                {"data", factory->Demolish(actor),
              }
            )
          );
        } catch (std::out_of_range e) {
          log::warn("Could not save actor {} their class name was unknown ({})", actorData.GetName(), actorData.GetCls());
          continue;
        }
      }
      return toml::value(array);
    }

    virtual void LoadData(const toml::value& table) override {
      for (auto value: table.as_array()) {
        RE::FormID formID = toml::find_or<RE::FormID>("formid", 0);
        std::string name = toml::find_or<std::string>("name", "");
        std::string clsName = toml::find_or<std::string>("cls", "");
        if (formID > 0) {
          auto& factory = this->factories.at(clsName);
          toml::value data = toml::find_or<toml::value>("data", toml::value());
          auto actor = factory->Build(data);
          this->actors.try_emplace(formID, std::move(actor));
        }
      }
    }

    // == EventListener
    virtual std::string DebugName() override {
      return "Actors";
    }

    virtual void DataReady() override {
      Cosave::RegisterForSerde("actors", this);
      Cosave::RegisterForDirectSerde(_byteswap_ulong('ACTD'), this);
    }

    // Links a toml class name to a cpp class
    template<T>
    static void RegisterActorFactory(std::string_view clsName) {
      T* factory = new T();

      Actors::GetSingleton().RegisterActorFactoryImpl(
        std::string(clsName),
        std::unique_ptr<ActorFactorBase>(factory);
      );
    }

    void RegisterActorFactoryImpl(std::string clsName, std::unique_ptr<ActorFactorBase> factory) {
      this->factories.try_emplace(clsName, std::move(factory));
    }

  private:
    std::unordered_map<RE::FormID, std::unique_ptr<GActor>> actors;
    std::unordered_map<std::string, std::unique_ptr<ActorFactorBase>> factories;
};

class ActorFactorBase {
public:
  virtual std::unique_ptr<GActor> Build(const toml::value& v) = 0;
  virtual toml::value Demolish(const GActor& built) = 0;
}

template<T>
class ActorFactor<T>: public ActorFactorBase {
  virtual std::unique_ptr<GActor> Build(const toml::value& v) override {
    T* built = new T();
    build->from_toml(v);
    return std::unique_ptr(build);
  }

  virtual toml::value Demolish(const GActor& built) override {
    return build.into_toml();
  }
}
