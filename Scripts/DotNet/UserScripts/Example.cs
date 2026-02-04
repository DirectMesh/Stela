using Stela;

public class Example
{
    void OnStart()
    {
        ScriptAPI.Log("C# Example Started");
    }

    void OnUpdate(float dt)
    {
        if (Input.KeyPressed(Keys.Space))
        {
            ScriptAPI.Log("Space pressed!");
        }
    }

    void OnShutdown()
    {
        ScriptAPI.Log("C# Example Shutdown");
    }
}
