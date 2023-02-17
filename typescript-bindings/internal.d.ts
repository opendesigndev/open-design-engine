
export declare const EngineSymbol: unique symbol;
export type Enum<Map extends { [key: string]: number }> = {
    [key in keyof Map]: { value: Map[key] };
};
export type EnumValue<T extends number> = { value: T };
