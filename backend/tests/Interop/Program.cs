NativeCoreTests.StateAndLifetime();
NativeCoreTests.InteractionQueue();
SessionTests.Lifecycle();
SessionTests.LifetimePolicy();
SessionTests.Capacity();
WeatherTests.Validation();
WeatherTests.Transformation();
WeatherTests.SessionValidityAndIsolation();
OutputTests.ReadingAndAcknowledgement();
OutputTests.CapacityAndRecovery();
OutputTests.WeatherValidity();
OutputTests.SessionDelivery();
OutputTests.UnsupportedActions();
ConversationTests.WeatherLocationConversation();
ConversationTests.SessionTextCommands();
await ConversationTests.SessionWeatherFetch();
await ConversationTests.SessionWeatherFailure();

Console.WriteLine("All interop tests passed.");
