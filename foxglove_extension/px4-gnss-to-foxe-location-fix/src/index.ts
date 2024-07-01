import { ExtensionContext } from "@foxglove/extension";

export function activate(extensionContext: ExtensionContext): void {

  extensionContext.registerMessageConverter({

    fromSchemaName: "estimator_global_position",
    toSchemaName: "foxglove.LocationFix",

    converter: (inputMessage: any) => {

      return {
        timestamp: inputMessage.timestamp,
        latitude: inputMessage.lat,
        longitude: inputMessage.lon,
        altitude: inputMessage.alt,
        frame_id: typeof inputMessage,
      };

    },

  });
}



