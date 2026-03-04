#version 450 core

flat in uint v_InstanceID;
uniform int instanceIndex;

out vec4 FragColor;

void main()
{
    // рисуем только выбранный инстанс
    if(v_InstanceID != uint(instanceIndex))
        discard;

    // можно убрать заливку внутренней части: оставляем только фронтальные грани
    if(!gl_FrontFacing)
        discard;

    // делаем очень прозрачный цвет, чтобы центр не перекрывался
    FragColor = vec4(1.0, 0.6, 0.0, 0.0); // почти полностью прозрачный
}